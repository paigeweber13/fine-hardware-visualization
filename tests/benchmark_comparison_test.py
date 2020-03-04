import argparse
import csv
import subprocess

benchmark_comparison_bin_location = '../bin/tests/benchmark-likwid-vs-manual'
flops_csv = './flops_comparison.csv'
mem_iter_csv = './mem_iter_comparison.csv'
mem_size_csv = './mem_size_comparison.csv'

flops_csv_header = ['manual_duration','manual_num_flops','manual_Mflops', \
                   'likwid_duration','likwid_num_flops','likwid_Mflops']
mem_csv_header = ['manual_duration','manual_data_size_mb', \
                 'manual_bandwidth_mb_per_s','likwid_duration', \
                 'likwid_data_size_mb','likwid_bandwitdh_mb_per_s']

def write_csv(filename, header, data):
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(data)

def run_tests():
    output = []
    num_flop_iterations = 100
    while num_flop_iterations < 10000000001:
        print('running flops with ' + str(num_flop_iterations) + ' iterations')
        child = subprocess.run(
            [benchmark_comparison_bin_location, '-f', str(num_flop_iterations),
             '-c'], stdout=subprocess.PIPE)
        output.append(child.stdout.decode().strip().split(','))
        num_flop_iterations *= 10

    write_csv(flops_csv, flops_csv_header, output)

    num_mem_iterations = 10
    output = []
    while num_mem_iterations < 100001:
        print('running flops with ' + str(num_flop_iterations) + ' iterations')
        child = subprocess.run(
            [benchmark_comparison_bin_location, '-f', str(num_flop_iterations),
             '-c'], stdout=subprocess.PIPE)
        output.append(child.stdout.decode().strip().split(','))
        num_mem_iterations *= 10

    write_csv(mem_iter_csv, mem_csv_header, output)

    mem_size = 5000
    output = []
    while mem_size < 1000001:
        print('running flops with ' + str(num_flop_iterations) + ' iterations')
        child = subprocess.run(
            [benchmark_comparison_bin_location, '-f', str(num_flop_iterations),
             '-c'], stdout=subprocess.PIPE)
        output.append(child.stdout.decode().strip().split(','))
        mem_size *= 2

    write_csv(mem_size_csv, mem_csv_header, output)

def plot_results():
    pass

def parse_cli_options():
    parser = argparse.ArgumentParser(
        description='Compare manual results with likwid results.')
    parser.add_argument('--run-tests', '-r', action='store_true')
    parser.add_argument('--plot-results', '-p', action='store_true')

    return parser.parse_args()

def main():
    write_csv(flops_csv, flops_csv_header, [])
    args = parse_cli_options()
    if args.run_tests:
        run_tests()
    if args.plot_results:
        plot_results()

if __name__ == '__main__':
    main()
