import argparse
import csv
import matplotlib.pyplot as plt
import pandas
import subprocess
import time

benchmark_comparison_bin_location = '../bin/tests/benchmark-likwid-vs-manual'
flops_file = './flops_comparison'
mem_iter_file = './mem_iter_comparison'
mem_size_file = './mem_size_comparison'

flops_csv_header = ['manual_duration','manual_num_flops','manual_Mflops', \
                   'likwid_duration','likwid_num_flops','likwid_Mflops']
mem_csv_header = ['manual_duration','manual_data_size_gb', \
                 'manual_bandwidth_mb_per_s','likwid_duration', \
                 'likwid_data_size_gb','likwid_bandwidth_mb_per_s']

def write_csv(filename, header, data):
    print('writing csv to ' + filename)
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(data)

def run_flop_tests():
    output = []
    num_flop_iterations = 100
    while num_flop_iterations < 100000000001:
        print('running flops with ' + str(num_flop_iterations) + ' iterations',
              end='', flush=True)
        start_time = time.time()
        child = subprocess.run(
            [benchmark_comparison_bin_location, '-f', str(num_flop_iterations),
             '-c'], stdout=subprocess.PIPE)
        elapsed_time = time.time() - start_time
        print(' {:.3f}s'.format(elapsed_time))
        output.append(child.stdout.decode().strip().split(','))
        num_flop_iterations *= 10

    write_csv(flops_file + '.csv', flops_csv_header, output)

def run_mem_iteration_tests():
    num_mem_iterations = 10
    mem_size = 10000
    output = []
    while num_mem_iterations < 500:
        print('running memory with ' + str(num_mem_iterations) + ' iterations'\
              ' and size ' + str(mem_size), end='', flush=True)
        start_time = time.time()
        child = subprocess.run(
            [benchmark_comparison_bin_location, '-m', str(num_mem_iterations),
             str(mem_size), '-c'], stdout=subprocess.PIPE)
        elapsed_time = time.time() - start_time
        print(' {:.3f}s'.format(elapsed_time))
        output.append(child.stdout.decode().strip().split(','))
        num_mem_iterations *= 2

    write_csv(mem_iter_file + '.csv', mem_csv_header, output)

def run_mem_size_tests():
    mem_size = 5000
    output = []

    start_num_iterations = 2000000
    minimum_num_iter = 3

    while mem_size < 4000001:
        num_mem_iterations = int(max(start_num_iterations/mem_size, 
                           minimum_num_iter))
        print('running memory with ' + str(num_mem_iterations) + ' iterations'\
              ' and size ' + str(mem_size), end='', flush=True)
        start_time = time.time()
        child = subprocess.run(
            [benchmark_comparison_bin_location, '-m', str(num_mem_iterations),
             str(mem_size), '-c'], stdout=subprocess.PIPE)
        elapsed_time = time.time() - start_time
        print(' {:.3f}s'.format(elapsed_time))
        output.append(child.stdout.decode().strip().split(','))
        mem_size *= 2

    write_csv(mem_size_file + '.csv', mem_csv_header, output)

def plot_results():
    # FLOPS: computation rate per number of computations

    # comparing likwid's reported values and manually calculated ones

    flop_data = pandas.read_csv(flops_file + '.csv')
    plt.figure(1, figsize=(10,7))
    plt.plot(flop_data['manual_num_flops'], flop_data['manual_Mflops'])
    plt.plot(flop_data['manual_num_flops'], flop_data['likwid_Mflops'])

    for xyt in zip(flop_data['manual_num_flops'],
                   flop_data['manual_Mflops'] * .7, 
                   flop_data['manual_duration']):
        plt.annotate("{:.3f}s".format(xyt[2]), xy=xyt[:2])

    plt.xscale('log')
    plt.yscale('log')
    plt.title('Comparing  computation rates reported by' \
        ' likwid and manually calculated computation rates')
    plt.xlabel('Number of SP FP operations')
    plt.ylabel('Computation Rate, Mflop/s')
    plt.legend(['Mflop/s calculated manually', 'Mflop/s as reported by likwid'])
    plt.savefig(flops_file + '.png')

    # MEMORY: bandwidth per amount of data transferred

    # comparing likwid's reported values and manually calculated ones

    mem_size_data = pandas.read_csv(mem_size_file + '.csv')
    plt.figure(2, figsize=(10,7))
    plt.plot(mem_size_data['manual_data_size_gb'], 
             mem_size_data['manual_bandwidth_mb_per_s'])
    plt.plot(mem_size_data['manual_data_size_gb'],
             mem_size_data['likwid_bandwidth_mb_per_s'])

    for xyt in zip(mem_size_data['manual_data_size_gb'],
                   mem_size_data['manual_bandwidth_mb_per_s'] * 1.01,
                   mem_size_data['manual_duration']):
        plt.annotate("{:.3f}s".format(xyt[2]), xy=xyt[:2])

    plt.xscale('log')
    plt.yscale('log')
    plt.title('Comparing bandwidth reported by likwid and manually ' \
        'calculated bandwidth')
    plt.xlabel('Amount of data transferred (GB)')
    plt.ylabel('Bandwidth, Mbytes/s')
    plt.legend(['Bandwidth calculated manually', 
                'Bandwidth as reported by likwid'])
    plt.savefig(mem_size_file + '.png')

    # MEMORY: amount of data transferred per trial

    # comparing likwid's reported values and manually calculated ones

    plt.figure(3, figsize=(10,7))
    trial_nums = range(len(mem_size_data['manual_data_size_gb']))
    plt.plot(trial_nums, mem_size_data['manual_data_size_gb'])
    plt.plot(trial_nums, mem_size_data['likwid_data_size_gb'])

    for xyt in zip(trial_nums,
                   mem_size_data['manual_data_size_gb'],
                   mem_size_data['manual_duration']):
        plt.annotate("{:.3f}s".format(xyt[2]), xy=(xyt[0] - 0.5, xyt[1]))

    plt.yscale('log')
    plt.title('Comparing amount of data transferred as reported by likwid ' \
        'and manually calculated amount of data transferred')
    plt.xlabel('Trial #')
    plt.ylabel('Amount of data transferred (GB)')
    plt.legend(['Amount of data calculated manually', 
                'Amoutn of data as reported by likwid'])
    plt.savefig(mem_size_file + '_sizes.png')


    # MEMORY: ratio of manually calculated data volume to likwid-reported data
    # volume

    plt.figure(4, figsize=(10,7))
    trial_nums = range(len(mem_size_data['manual_data_size_gb']))
    plt.plot(trial_nums, 
             mem_size_data['manual_data_size_gb']/
             mem_size_data['likwid_data_size_gb'])

    for xyt in zip(trial_nums,
                   mem_size_data['manual_data_size_gb']/
                   mem_size_data['likwid_data_size_gb'],
                   mem_size_data['manual_duration']):
        plt.annotate("{:.3f}s".format(xyt[2]), xy=(xyt[0] + 0.1, xyt[1]))

    plt.axhline(y=1)
    plt.annotate("1:1", (0, 1.1))
    # plt.yscale('log')
    plt.title('ratio of manually calculated data volume to likwid-reported ' \
              'data volume')
    plt.xlabel('Trial #')
    plt.ylabel('Ratio of manually calculated volume to likwid-reported volume')
    plt.savefig(mem_size_file + '_size_ratio.png')

    # plt.show()

def parse_cli_options():
    parser = argparse.ArgumentParser(
        description='Compare manual results with likwid results.')
    parser.add_argument('--run-flop-tests', '-f', action='store_true')
    parser.add_argument('--run-mem-iteration-tests', '-i', action='store_true')
    parser.add_argument('--run-mem-size-tests', '-s', action='store_true')
    parser.add_argument('--plot-results', '-p', action='store_true')

    return parser.parse_args()

def main():
    args = parse_cli_options()
    if args.run_flop_tests:
        run_flop_tests()
    if args.run_mem_iteration_tests:
        run_mem_iteration_tests()
    if args.run_mem_size_tests:
        run_mem_size_tests()
    if args.plot_results:
        plot_results()

if __name__ == '__main__':
    main()
