#!/usr/bin/python3

# dependencies (pip install --user <dependencies>)
#  - matplotlib
#  - pandas

import matplotlib.pyplot as plt
import pandas

basic_data_file = 'data/basic_likwid_2020-05-01_1408.csv'
block_optimized_data_file = 'data/block_optimized_likwid_2020-05-01_1448.csv'

basic_data = pandas.read_csv(basic_data_file)
block_optimized_data = pandas.read_csv(block_optimized_data_file)

# print(block_optimized_data.loc(v))

print('basic_data num rows:', basic_data.shape[0])
print('block_optimized_data num rows:', block_optimized_data.shape[0])

print('selecting only those where n = 67108864, excluding "all_region" group')

basic_data = basic_data.loc[
    (basic_data['region'] == 'poly') & 
    (basic_data['n'] == 67108864)
    ]
print('basic_data num rows:', basic_data.shape[0])

block_optimized_data = block_optimized_data.loc[
    (block_optimized_data['region'] == 'poly_block') & 
    (block_optimized_data['n'] == 67108864)
    ]
print('block_optimized_data num rows:', block_optimized_data.shape[0])

metrics = [
    'saturation SP [MFLOP/s]',
    # 'saturation DP [MFLOP/s]',
    # 'saturation L2 bandwidth [MBytes/s]',
    # 'saturation L3 bandwidth [MBytes/s]',
    'saturation Memory bandwidth [MBytes/s]',
]

print('discarding data where any ratio is above 1, because that '
    'doesn\'t make sense')

for metric in metrics:
    basic_data = basic_data.loc[
        basic_data[metric] <= 1.1
        ]
    block_optimized_data = block_optimized_data.loc[
        block_optimized_data[metric] <= 1.1
        ]

print('basic_data num rows:', basic_data.shape[0])
print('block_optimized_data num rows:', block_optimized_data.shape[0])

plt.figure(figsize=(10, 7))

for metric in metrics:
    plt.plot(basic_data['degree'], basic_data[metric])

plt.xscale('log')
plt.legend(metrics)
plt.title("Basic code saturation by degree")
plt.xlabel("Degree")
plt.ylabel("Saturation level")

plt.figure(figsize=(10, 7))

for metric in metrics:
    plt.plot(block_optimized_data['degree'], block_optimized_data[metric])

plt.xscale('log')
plt.legend(metrics)
plt.title("Block optimized code saturation by degree")
plt.xlabel("Degree")
plt.ylabel("Saturation level")

#### saturation level comparisons between kernels ####

kernel_comparison_metrics_basic = [
    ('saturation SP [MFLOP/s]', 'tab:cyan'),
    ('saturation Memory bandwidth [MBytes/s]', 'tab:orange'),
]

kernel_comparison_metrics_block_optimized = [
    ('saturation SP [MFLOP/s]', 'tab:purple'),
    ('saturation Memory bandwidth [MBytes/s]',
        'tab:red'),
]

legend = []
for metric in kernel_comparison_metrics_basic:
    legend.append('basic code - ' + metric[0])

for metric in kernel_comparison_metrics_block_optimized:
    legend.append('block optimized code - ' + metric[0])

plt.figure(figsize=(10, 7))

for metric, color in kernel_comparison_metrics_basic:
    plt.plot(basic_data['degree'], basic_data[metric], color=color)

for metric, color in kernel_comparison_metrics_block_optimized:
    plt.plot(block_optimized_data['degree'], 
        block_optimized_data[metric], 
        color=color)

plt.xscale('log')
plt.legend(legend)
plt.title("Basic and optimized code comparison")
plt.xlabel("Degree")
plt.ylabel("Saturation level")

plt.show()
