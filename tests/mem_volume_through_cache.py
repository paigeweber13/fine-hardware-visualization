import argparse
import csv
import matplotlib.pyplot as plt
import pandas
import subprocess
import time

fhv_bin_location = '../bin/fhv'
results_file = './mem_volume_through_cache'

def write_csv(filename, header, data):
    print('writing csv to ' + filename)
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(data)

def get_header():
    child = subprocess.run(
        [fhv_bin_location, '--print-csv-header'], stdout=subprocess.PIPE)
    return child.stdout.decode().strip().split(',')

def run_tests():
    output = []
    data_size_kb = 10
    start_num_iterations = 200000
    minimum_num_iter = 3

    while data_size_kb < 1000001:
        num_iter = int(max(start_num_iterations/data_size_kb, 
                           minimum_num_iter))
        command = [
            fhv_bin_location, '--benchmark-cache-and-memory', 
            str(num_iter), str(data_size_kb),
            '-c']
        # print('running command:', ' '.join(command))
        print('running memory with data size ' + str(data_size_kb) + ' and '
              'number of iterations ' + str(num_iter),
              end='', flush=True)
        start_time = time.time()
        child = subprocess.run(command, stdout=subprocess.PIPE)
        elapsed_time = time.time() - start_time
        print(' {:.3f}s'.format(elapsed_time))
        output.append(child.stdout.decode().strip().split(','))
        data_size_kb *= 10

    write_csv(results_file + '.csv', get_header(), output)

def plot_results():
    data =  pandas.read_csv(results_file + '.csv')
    plt.figure(1, figsize=(10,7))
    trial_nums = range(len(data['L2D evict data volume [GBytes]']))
    plt.plot(trial_nums, data['L2D evict data volume [GBytes]'],
             color='midnightblue')
    plt.plot(trial_nums, data['L2D load data volume [GBytes]'],
             color='cornflowerblue')
    plt.plot(trial_nums, data['L3 evict data volume [GBytes]'],
             color='darkred')
    plt.plot(trial_nums, data['L3 load data volume [GBytes]'],
             color='red')
    plt.plot(trial_nums, data['Memory evict data volume [GBytes]'],
             color='darkgreen')
    plt.plot(trial_nums, data['Memory load data volume [GBytes]'],
             color='seagreen')
    print(data)
    print(data['Memory evict data volume [GBytes]'])
    print(data['Memory load data volume [GBytes]'])

    # plt.xscale('log')
    # plt.yscale('log')
    plt.title('Comparing read and write volumes through cache and memory')
    plt.xlabel('Trial number')
    plt.ylabel('Memory volume (GBytes)')
    plt.legend([
        'L2D evict data volume [GBytes]',
        'L2D load data volume [GBytes]',
        'L3 evict data volume [GBytes]',
        'L3 load data volume [GBytes]',
        'Memory evict data volume [GBytes]',
        'Memory load data volume [GBytes]',
    ])

    plt.savefig(results_file + '.png')

    plt.show()

def parse_cli_options():
    parser = argparse.ArgumentParser(
        description='Compare memory volume through cache and ram')
    parser.add_argument('--run-tests', '-r', action='store_true')
    parser.add_argument('--plot-results', '-p', action='store_true')

    return parser.parse_args()

def main():
    args = parse_cli_options()

    if args.run_tests:
        run_tests()
    if args.plot_results:
        plot_results()

if __name__ == '__main__':
    main()
