import re
import sys
import numpy as np
from math import log2

# read log file from argument
if len(sys.argv) != 5:
  print(
      'Usage: python collect.py <mp24_log_file> <symbol_log_file>'
      ' <kpz21_log_file> <mono_log_file>'
  )
  sys.exit(1)

# has the form index : [value]
noise_index_to_value = {}


def analyse_log(log_file_name):
  try:
    with open(log_file_name, 'r') as file:
      log_data = file.read()
  except FileNotFoundError:
    print(f"Error: File '{log_file_name}' not found.")
    sys.exit(1)

  noise_bounds = []

  prev_noise = None
  for line in log_data.splitlines():
    noise_match = re.search(r'noise:\s*([\d.]+)', line)
    noise_bound_match = re.search(r'noise bound:\s*([\d.]+)', line)
    if noise_match:
      prev_noise = float(noise_match.group(1))
    if noise_bound_match:
      noise_bound = float(noise_bound_match.group(1))
      if prev_noise is not None:
        if noise_bound not in noise_bounds:
          noise_bounds.append(noise_bound)
        index = noise_bounds.index(noise_bound)
        if index not in noise_index_to_value:
          noise_index_to_value[index] = []
        noise_index_to_value[index].append(prev_noise)

  return noise_bounds


mp24_noise_bounds = analyse_log(sys.argv[1])
symbol_noise_bounds = analyse_log(sys.argv[2])
# kpz21_noise_bounds = analyse_log(sys.argv[3])
# mono_noise_bounds = analyse_log(sys.argv[4])

x_axis = range(1, len(noise_index_to_value) + 1)

# for each bound, calculate the max and min noise
max_noise = {}
min_noise = {}
average_noise = {}
percentile_75_noise = {}
percentile_25_noise = {}

for index, noises in noise_index_to_value.items():
  max_noise[index] = max(noises)
  min_noise[index] = min(noises)
  # noises contains log values, the average is calculated in another way
  original_noises = [2**noise for noise in noises]
  percentile_75_noise[index] = log2(np.percentile(original_noises, 75))
  percentile_25_noise[index] = log2(np.percentile(original_noises, 25))
  # average_noise[index] = log2(sum(original_noises) / len(original_noises))
  # median instead of average
  average_noise[index] = log2(np.percentile(original_noises, 50))

# Plot the noise and noise bound
import matplotlib.pyplot as plt

# Prepare data for plotting
max_noises = [max_noise[index] for index in max_noise]
min_noises = [min_noise[index] for index in min_noise]
average_noises = [average_noise[index] for index in average_noise]

# minus average for max and min and other bounds
max_noises = [
    (max_noise[index] - average_noise[index]) / average_noise[index]
    for index in average_noise
]
min_noises = [
    (min_noise[index] - average_noise[index]) / average_noise[index]
    for index in average_noise
]
percentile_75_noises = [
    (percentile_75_noise[index] - average_noise[index]) / average_noise[index]
    for index in average_noise
]
percentile_25_noises = [
    (percentile_25_noise[index] - average_noise[index]) / average_noise[index]
    for index in average_noise
]
mp24_noise_bounds = [
    (mp24_noise_bounds[index] - average_noise[index]) / average_noise[index]
    for index in average_noise
]
symbol_noise_bounds = [
    (symbol_noise_bounds[index] - average_noise[index]) / average_noise[index]
    for index in average_noise
]
# kpz21_noise_bounds = [
#    (kpz21_noise_bounds[index] - average_noise[index])/average_noise[index] for index in average_noise
# ]
# mono_noise_bounds = [
#    (mono_noise_bounds[index] - average_noise[index])/average_noise[index] for index in average_noise
# ]

# Plot max and min noise for each bound
plt.figure(figsize=(10, 6))
plt.plot(x_axis, max_noises, label='Max Noise', marker='v')
plt.plot(x_axis, min_noises, label='Min Noise', marker='^')
plt.plot(x_axis, percentile_75_noises, label='75th Percentile', marker='s')
plt.plot(x_axis, percentile_25_noises, label='25th Percentile', marker='p')
plt.plot(x_axis, mp24_noise_bounds, label='MP24', marker='1')
plt.plot(x_axis, symbol_noise_bounds, label='Symbol', marker='2')
# plt.plot(x_axis, kpz21_noise_bounds, label='KPZ21', marker='3')
# plt.plot(x_axis, mono_noise_bounds, label='Mono', marker='4')

plt.legend()
plt.xlabel('Index')
plt.ylabel('Noise Diff')
plt.grid(True)
plt.savefig('noise.pdf')
