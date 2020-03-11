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
mem_csv_header = ['manual_duration','manual_data_size_mb', \
                 'manual_bandwidth_mb_per_s','likwid_duration', \
                 'likwid_data_size_mb','likwid_bandwitdh_mb_per_s']

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
    num_mem_iterations = 20
    mem_size = 5000
    output = []
    while mem_size < 1000001:
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
    flop_data = pandas.read_csv(flops_file + '.csv')
    plt.figure(figsize=(10,7))
    plt.plot(flop_data['manual_num_flops'], flop_data['manual_Mflops'])
    plt.plot(flop_data['manual_num_flops'], flop_data['likwid_Mflops'])

    for xyt in zip(flop_data['manual_num_flops'],
                   flop_data['manual_Mflops'] * .7, 
                   flop_data['manual_duration']):
        plt.annotate("{:.3f}s".format(xyt[2]), xy=xyt[:2])

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of SP FP operations')
    plt.ylabel('Computation Rate, Mflop/s')
    plt.legend(['Mflop/s calculated manually', 'Mflop/s as reported by likwid'])
    plt.show()
    plt.savefig(flops_file + '.png')

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
