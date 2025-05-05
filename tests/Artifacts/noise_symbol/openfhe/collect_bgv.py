import re
import sys
import numpy as np
from math import log2

# mult_seq_dep_8_bgv_log_files = [
#    'mult_seq_dep_8_bgv_symbol_logN14.log',
#    'mult_seq_dep_8_bgv_mp24_logN14.log',
#    'mult_seq_dep_8_bgv_kpz21_logN14.log',
#    'mult_seq_dep_8_bgv_mono_logN14.log',
# ]
#
# mult_seq_indep_8_bgv_log_files = [
#    'mult_seq_indep_8_bgv_symbol_logN14.log',
#    'mult_seq_indep_8_bgv_mp24_logN14.log',
#    'mult_seq_indep_8_bgv_kpz21_logN14.log',
#    'mult_seq_indep_8_bgv_mono_logN14.log',
# ]

mult_seq_dep_12_bgv_log_files = [
    'mult_seq_dep_12_bgv_symbol_logN14.log',
    'mult_seq_dep_12_bgv_symbol-gaussian_logN14.log',
    'mult_seq_dep_12_bgv_symbol-laplacian_logN14.log',
]

mult_seq_indep_12_bgv_log_files = [
    'mult_seq_indep_12_bgv_symbol_logN14.log',
    'mult_seq_indep_12_bgv_symbol-gaussian_logN14.log',
    'mult_seq_indep_12_bgv_symbol-laplacian_logN14.log',
]


# has the form index : [value]
def analyse_log(noise_index_to_value, log_file_name):
  try:
    with open(log_file_name, 'r') as file:
      log_data = file.read()
  except FileNotFoundError:
    print(f"Error: File '{log_file_name}' not found.")
    sys.exit(1)

  # index : bound
  noise_bounds = {}

  count = 0

  for line in log_data.splitlines():
    count_match = re.search(r'count:\s*([\d.]+)', line)
    if count_match:
      count = int(count_match.group(1))

    noise_match = re.search(r'noise:\s*([\d.]+)', line)
    if noise_match:
      noise = float(noise_match.group(1))
      if count not in noise_index_to_value:
        noise_index_to_value[count] = []
      noise_index_to_value[count].append(noise)

    noise_bound_match = re.search(r'noise bound:\s*([\d.]+)', line)
    if noise_bound_match:
      noise_bound = float(noise_bound_match.group(1))
      noise_bounds[count] = noise_bound

  return noise_bounds


# noise_dep_log14_index_to_value = {}
# symbol_noise_bounds = analyse_log(
#     noise_dep_log14_index_to_value, mult_seq_dep_8_bgv_log_files[0]
# )
# mp24_noise_bounds = analyse_log(
#     noise_dep_log14_index_to_value, mult_seq_dep_8_bgv_log_files[1]
# )
# kpz21_noise_bounds = analyse_log(
#     noise_dep_log14_index_to_value, mult_seq_dep_8_bgv_log_files[2]
# )
# mono_noise_bounds = analyse_log(
#     noise_dep_log14_index_to_value, mult_seq_dep_8_bgv_log_files[3]
# )
#
# noise_indep_log14_index_to_value = {}
# symbol_indep_noise_bounds = analyse_log(
#     noise_indep_log14_index_to_value, mult_seq_indep_8_bgv_log_files[0]
# )
# mp24_indep_noise_bounds = analyse_log(
#     noise_indep_log14_index_to_value, mult_seq_indep_8_bgv_log_files[1]
# )
# kpz21_indep_noise_bounds = analyse_log(
#     noise_indep_log14_index_to_value, mult_seq_indep_8_bgv_log_files[2]
# )
# mono_indep_noise_bounds = analyse_log(
#     noise_indep_log14_index_to_value, mult_seq_indep_8_bgv_log_files[3]
# )

noise_dep_log14_index_to_value = {}
symbol_noise_bounds = analyse_log(
    noise_dep_log14_index_to_value, mult_seq_dep_12_bgv_log_files[0]
)
symbol_gaussian_noise_bounds = analyse_log(
    noise_dep_log14_index_to_value, mult_seq_dep_12_bgv_log_files[1]
)
symbol_laplacian_noise_bounds = analyse_log(
    noise_dep_log14_index_to_value, mult_seq_dep_12_bgv_log_files[2]
)

noise_indep_log14_index_to_value = {}
symbol_indep_noise_bounds = analyse_log(
    noise_indep_log14_index_to_value, mult_seq_indep_12_bgv_log_files[0]
)
symbol_gaussian_indep_noise_bounds = analyse_log(
    noise_indep_log14_index_to_value, mult_seq_indep_12_bgv_log_files[1]
)
symbol_laplacian_indep_noise_bounds = analyse_log(
    noise_indep_log14_index_to_value, mult_seq_indep_12_bgv_log_files[2]
)


def process_index_to_value(is_indep, index_to_value):
  # for each bound, calculate the max and min noise
  max_noise = {}
  min_noise = {}
  average_noise = {}
  percentile_75_noise = {}
  percentile_25_noise = {}

  for index, noises in index_to_value.items():
    # skip the first 12 indexes if is indep
    if is_indep:
      if index < 11:
        continue
      index -= 11
    max_noise[index] = max(noises)
    min_noise[index] = min(noises)
    # noises contains log values, the average is calculated in another way
    original_noises = [2**noise for noise in noises]
    percentile_75_noise[index] = log2(np.percentile(original_noises, 75))
    percentile_25_noise[index] = log2(np.percentile(original_noises, 25))
    # average_noise[index] = log2(sum(original_noises) / len(original_noises))
    # median instead of average
    average_noise[index] = log2(np.percentile(original_noises, 50))

  # to_array
  max_noise = [max_noise[index] for index in max_noise]
  min_noise = [min_noise[index] for index in min_noise]
  average_noise = [average_noise[index] for index in average_noise]
  percentile_75_noise = [
      percentile_75_noise[index] for index in percentile_75_noise
  ]
  percentile_25_noise = [
      percentile_25_noise[index] for index in percentile_25_noise
  ]

  return (
      max_noise,
      min_noise,
      average_noise,
      percentile_75_noise,
      percentile_25_noise,
  )


def process_bounds(is_dep, bounds):
  bounds_array = []
  for index, bound in bounds:
    # skip the first 11 bound
    if is_dep:
      if index < 11:
        continue
    bounds_array.append(bound)
  return bounds_array


def base_line(array, base_array):
  # for each element in array, minus base
  return [element - base_array[index] for index, element in enumerate(array)]


(
    max_dep_noise,
    min_dep_noise,
    average_dep_noise,
    percentile_75_dep_noise,
    percentile_25_dep_noise,
) = process_index_to_value(False, noise_dep_log14_index_to_value)
(
    max_indep_noise,
    min_indep_noise,
    average_indep_noise,
    percentile_75_indep_noise,
    percentile_25_indep_noise,
) = process_index_to_value(True, noise_indep_log14_index_to_value)

symbol_noise_bounds_array = process_bounds(False, symbol_noise_bounds.items())
symbol_indep_noise_bounds_array = process_bounds(
    True, symbol_indep_noise_bounds.items()
)

symbol_gaussian_noise_bounds_array = process_bounds(
    False, symbol_gaussian_noise_bounds.items()
)
symbol_gaussian_indep_noise_bounds_array = process_bounds(
    True, symbol_gaussian_indep_noise_bounds.items()
)

symbol_laplacian_noise_bounds_array = process_bounds(
    False, symbol_laplacian_noise_bounds.items()
)
symbol_laplacian_indep_noise_bounds_array = process_bounds(
    True, symbol_laplacian_indep_noise_bounds.items()
)
# mp24_noise_bounds_array = process_bounds(False, mp24_noise_bounds.items())
# kpz21_noise_bounds_array = process_bounds(False, kpz21_noise_bounds.items())
# mono_noise_bounds_array = process_bounds(False, mono_noise_bounds.items())

# base on average_indep_noise
symbol_noise_bounds_array_based = base_line(
    symbol_noise_bounds_array, average_indep_noise
)
symbol_indep_noise_bounds_array_based = base_line(
    symbol_indep_noise_bounds_array, average_indep_noise
)

symbol_gaussian_noise_bounds_array_based = base_line(
    symbol_gaussian_noise_bounds_array, average_indep_noise
)
symbol_gaussian_indep_noise_bounds_array_based = base_line(
    symbol_gaussian_indep_noise_bounds_array, average_indep_noise
)

symbol_laplacian_noise_bounds_array_based = base_line(
    symbol_laplacian_noise_bounds_array, average_indep_noise
)
symbol_laplacian_indep_noise_bounds_array_based = base_line(
    symbol_laplacian_indep_noise_bounds_array, average_indep_noise
)
# mp24_noise_bounds_array_based = base_line(
#    mp24_noise_bounds_array, average_indep_noise
# )
# kpz21_noise_bounds_array_based = base_line(
#    kpz21_noise_bounds_array, average_indep_noise
# )
# mono_noise_bounds_array_based = base_line(
#    mono_noise_bounds_array, average_indep_noise
# )

# base dep on indep
average_dep_noise = base_line(average_dep_noise, average_indep_noise)
max_dep_noise = base_line(max_dep_noise, average_indep_noise)
min_dep_noise = base_line(min_dep_noise, average_indep_noise)
max_indep_noise = base_line(max_indep_noise, average_indep_noise)
min_indep_noise = base_line(min_indep_noise, average_indep_noise)
# making is all zero
average_indep_noise = base_line(average_indep_noise, average_indep_noise)

# Plot the noise and noise bound
import matplotlib.pyplot as plt

x_axis = range(1, 12 + 1)

# Plot max and min noise for each bound
plt.figure(figsize=(10, 6))
plt.plot(x_axis, average_indep_noise, label='Median Ind', marker=',')
plt.fill_between(x_axis, min_indep_noise, max_indep_noise, alpha=0.2)
plt.plot(x_axis, average_dep_noise, label='Median Dep', marker='.')
plt.fill_between(x_axis, min_dep_noise, max_dep_noise, alpha=0.2)
plt.plot(
    x_axis,
    symbol_indep_noise_bounds_array_based,
    label='Symbol Ind',
    marker='^',
)
plt.plot(
    x_axis, symbol_noise_bounds_array_based, label='Symbol Dep', marker='v'
)

plt.plot(
    x_axis,
    symbol_gaussian_indep_noise_bounds_array_based,
    label='Symbol Ind Gaussian',
    marker='^',
)
plt.plot(
    x_axis,
    symbol_gaussian_noise_bounds_array_based,
    label='Symbol Dep Gaussian',
    marker='v',
)

plt.plot(
    x_axis,
    symbol_laplacian_indep_noise_bounds_array_based,
    label='Symbol Ind Laplacian',
    marker='^',
)
plt.plot(
    x_axis,
    symbol_laplacian_noise_bounds_array_based,
    label='Symbol Dep Laplacian',
    marker='v',
)
# plt.plot(x_axis, mp24_noise_bounds_array_based, label='MP24', marker='*')
# plt.plot(x_axis, mono_noise_bounds_array_based, label='Mono', marker='s')
# plt.plot(x_axis, kpz21_noise_bounds_array_based, label='KPZ21', marker='p')

plt.legend()
plt.xlabel('Index')
plt.ylabel('Noise Diff')
plt.grid(True)
plt.savefig('noise.pdf')
