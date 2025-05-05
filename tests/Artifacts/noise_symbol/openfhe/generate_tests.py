import os
import pathlib
import subprocess

#
# Templates
#

test_template = """
#include "common_header.cpp.inc"

// Generated headers (block clang-format from messing up order)
#include "{test_name}_{scheme}_{noise_model}_{logN}.h"

// Debug handler
#include "{scheme}_debug.cpp.inc"

// main (block clang-format from messing up order)
#include "{test_main_name}_main.cpp.inc"
"""

executable_name_template = "{test_name}_{scheme}_{noise_model}_{logN}"
output_file_name_template = "{test_name}_{scheme}_{noise_model}_{logN}.cpp"
compiled_file_name_template = "{test_name}_{scheme}_{noise_model}_{logN}.h"
mlir_file_name_template = "{mlir_name}_{logN}.mlir"

#
# Compiler call
#


def compile(src, out, **kargs):
  noise_model = kargs.get("noise_model")
  scheme = kargs.get("scheme")
  ring_dim = kargs.get("ring_dim")
  mul_depth = kargs.get("mul_depth")
  test_name = kargs.get("test_name")

  heir_opt = "bazel run //tools:heir-opt --"
  heir_translate = "bazel run //tools:heir-translate --"

  need_operation_balancer = "tree" in test_name

  heir_opt_flags = [
      "--wrap-generic",
      ("--operation-balancer" if need_operation_balancer else "--cse"),
      "--cse",
      # for skipping modulus switching
      "--annotate-module=backend=openfhe",
      f"--secret-insert-mgmt-{scheme}",
      # for actually running the validate-noise pass
      "--annotate-module=backend=lattigo",
      (
          "--validate-noise=annotate-noise-bound=true"
          f" model={scheme}-noise-{noise_model}"
      ),
      "--secret-distribute-generic",
      "--secret-to-bgv",
      "--lwe-add-client-interface",
      "--bgv-to-lwe",
      "--lwe-add-debug-port",
      "--lwe-to-openfhe",
      "--canonicalize",
      "--cse",
      "--openfhe-configure-crypto-context=insecure=true"
      f" ring-dim={ring_dim} mul-depth={mul_depth} scaling-mod-size=60"
      + (
          " first-mod-size=60 scaling-technique-fixed-manual=true"
          if scheme == "bgv"
          else ""
      ),
  ]

  heir_translate_flags = [
      "--emit-openfhe-pke",
      "--openfhe-include-type=embedded",
  ]

  src = os.path.realpath(src)
  out = os.path.realpath(out)

  # create a temporary file
  temp_file = out + ".mlir"

  try:
    subprocess.run(
        [*heir_opt.split(), src, *heir_opt_flags, "-o", temp_file], check=True
    )
    subprocess.run(
        [*heir_translate.split(), temp_file, *heir_translate_flags, "-o", out],
        check=True,
    )
  except subprocess.CalledProcessError as e:
    print(f"Compilation failed: {e}")

  # remove the temporary file
  # if os.path.exists(temp_file):
  #    os.remove(temp_file)


#
# Generator
#

schemes = ["bgv", "bfv"]
# noise_models_bgv = ["symbol", "mp24", "kpz21", "mono"]
noise_models_bgv = ["symbol", "symbol-gaussian", "symbol-laplacian"]
# noise_models_bfv = ["symbol", "bmcm23", "kpz21"]
noise_models_bfv = ["symbol", "symbol-gaussian", "symbol-laplacian"]
test_names = [
    #    "mult_seq_dep_8",
    #    "mult_tree_dep_8",
    #    "mult_seq_indep_8",
    #    "mult_tree_indep_8",
    "mult_seq_dep_12",
    "mult_seq_indep_12",
]
logNs = [
    #    "logN13",
    "logN14",
    #    "logN15",
]

mlir_name_mapping = {
    "mult_seq_dep_8": "mult_dep_8",
    "mult_tree_dep_8": "mult_dep_8",
    "mult_seq_indep_8": "mult_indep_8",
    "mult_tree_indep_8": "mult_indep_8",
    "mult_seq_dep_12": "mult_dep_12",
    "mult_seq_indep_12": "mult_indep_12",
}

test_main_name_mapping = {
    "mult_seq_dep_8": "mult_dep",
    "mult_tree_dep_8": "mult_dep",
    "mult_seq_indep_8": "mult_indep_8",
    "mult_tree_indep_8": "mult_indep_8",
    "mult_seq_dep_12": "mult_dep",
    "mult_seq_indep_12": "mult_indep_12",
}

ring_dim_mapping = {
    "logN13": 8192,
    "logN14": 16384,
    "logN15": 32768,
}

mul_depth_mapping = {
    "mult_seq_dep_8": 8,
    "mult_tree_dep_8": 8,
    "mult_seq_indep_8": 8,
    "mult_tree_indep_8": 8,
    "mult_seq_dep_12": 12,
    "mult_seq_indep_12": 12,
}


def main():
  executable_list = []
  pwd = pathlib.Path.cwd()
  for scheme in schemes:
    if scheme == "bgv":
      noise_models = noise_models_bgv
    else:
      noise_models = noise_models_bfv
    for noise_model in noise_models:
      for test_name in test_names:
        for logN in logNs:
          kargs = {
              "noise_model": noise_model,
              "scheme": scheme,
              "test_name": test_name,
              "test_main_name": test_main_name_mapping[test_name],
              "logN": logN,
              "mlir_name": mlir_name_mapping[test_name],
              "ring_dim": ring_dim_mapping[logN],
              "mul_depth": mul_depth_mapping[test_name],
          }
          output_file_name = output_file_name_template.format(**kargs)
          compiled_file_name = compiled_file_name_template.format(**kargs)
          mlir_file_name = mlir_file_name_template.format(**kargs)

          mlir_path = pwd / "common" / mlir_file_name
          compiled_path = pwd / "build" / compiled_file_name
          output_path = pwd / "build" / output_file_name

          compile(mlir_path, compiled_path, **kargs)
          test_content = test_template.format(**kargs)
          with open(output_path, "w") as f:
            f.write(test_content)
          executable_name = executable_name_template.format(**kargs)
          executable_list.append(executable_name)
  # write the executable list to a file
  with open(pwd / "build" / "executable_list.txt", "w") as f:
    for executable in executable_list:
      f.write(executable + "\n")


main()
