import argparse


def generate_mult_dep_mlir(k):
  mlir_code = """
module {
  func.func @mult_dep(%0: i16 {secret.secret}) -> i16 {
"""
  for i in range(1, k):
    mlir_code += f"    %{i} = arith.muli %{i-1}, %0 : i16\n"

  mlir_code += f"    return %{k-1} : i16\n"
  mlir_code += """  }
}
"""
  return mlir_code


def main():
  parser = argparse.ArgumentParser(
      description="Generate MLIR to multiply k i16 together."
  )
  parser.add_argument("k", type=int, help="Number of i16 multiplications.")
  args = parser.parse_args()

  if args.k < 1:
    print("k must be at least 1.")
    return

  mlir_code = generate_mult_dep_mlir(args.k)
  print(mlir_code)


if __name__ == "__main__":
  main()
