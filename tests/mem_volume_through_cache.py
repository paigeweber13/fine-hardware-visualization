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
    num_iter = 2000
    # start_num_iterations = 200000
    # minimum_num_iter = 3

    while data_size_kb < 15000:
        # num_iter = int(max(start_num_iterations/data_size_kb, 
        #                    minimum_num_iter))
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
        data_size_kb *= 2

    write_csv(results_file + '.csv', get_header(), output)

def plot_results():
    data =  pandas.read_csv(results_file + '.csv')

    ### RAW VOLUME - L2
    plt.figure(1, figsize=(10,7))

    # trial_nums = range(len(data['L2D evict data volume [GBytes]']))
    plt.plot(data['Single iteration size'], 
             data['L2D load data volume [GBytes]'],
             color='cornflowerblue')
    plt.plot(data['Single iteration size'], 
             data['L2D evict data volume [GBytes]'],
             color='midnightblue')

    plt.xscale('log')
    plt.yscale('log')
    plt.title('Comparing read and write volumes through L2 cache')
    plt.xlabel('Size of one iteration (KBytes)')
    plt.ylabel('Memory volume (GBytes)')
    plt.legend([
        'L2D load data volume [GBytes]',
        'L2D evict data volume [GBytes]',
    ])

    plt.savefig(results_file + '_l2.png')

    ### RAW VOLUME - L3
    plt.figure(2, figsize=(10,7))

    # trial_nums = range(len(data['L2D evict data volume [GBytes]']))
    plt.plot(data['Single iteration size'], 
             data['L3 load data volume [GBytes]'],
             color='red')
    plt.plot(data['Single iteration size'], 
             data['L3 evict data volume [GBytes]'],
             color='darkred')

    plt.xscale('log')
    plt.yscale('log')
    plt.title('Comparing read and write volumes through L3 cache')
    plt.xlabel('Size of one iteration (KBytes)')
    plt.ylabel('Memory volume (GBytes)')
    plt.legend([
        'L3 load data volume [GBytes]',
        'L3 evict data volume [GBytes]',
    ])

    plt.savefig(results_file + '_l3.png')
    
    ### RAW VOLUME - Memory
    plt.figure(3, figsize=(10,7))

    plt.plot(data['Single iteration size'], 
             data['Memory load data volume [GBytes]'],
             color='seagreen')
    plt.plot(data['Single iteration size'], 
             data['Memory evict data volume [GBytes]'],
             color='darkgreen')

    plt.xscale('log')
    plt.yscale('log')
    plt.title('Comparing read and write volumes through memory')
    plt.xlabel('Size of one iteration (KBytes)')
    plt.ylabel('Memory volume (GBytes)')
    plt.legend([
        'Memory load data volume [GBytes]',
        'Memory evict data volume [GBytes]',
    ])

    plt.savefig(results_file + '_memory.png')

    ### RATIOS
    plt.figure(4, figsize=(10,7))
    l2_ratios = data['L2D load data volume [GBytes]'] \
                / data['L2D evict data volume [GBytes]']
    l3_ratios = data['L3 load data volume [GBytes]'] \
                / data['L3 evict data volume [GBytes]']
    mem_ratios = data['Memory load data volume [GBytes]'] \
                 / data['Memory evict data volume [GBytes]']

    # cap ratios
    max_ratio = 4
    l2_ratios = l2_ratios.clip(0,max_ratio)
    l3_ratios = l3_ratios.clip(0,max_ratio)
    mem_ratios = mem_ratios.clip(0,max_ratio)


    # print(data['L2D load data volume [GBytes]'])
    # print(data['L2D evict data volume [GBytes]'])
    # print('l2 ratios: \n', l2_ratios)
    # print('l3 ratios: \n', l3_ratios)
    # print('mem ratios: \n', mem_ratios)
    print('NOTE: ratios are capped at {:d}. Some are very large (in the ' \
          'order of 10^2) but are capped for visibility'.format(max_ratio))

    plt.plot(data['Single iteration size'], l2_ratios,
             color='midnightblue')
    plt.plot(data['Single iteration size'], l3_ratios,
             color='darkred')
    plt.plot(data['Single iteration size'], mem_ratios,
             color='darkgreen')

    plt.title('Comparing ratios of reads:writes through cache and memory')
    plt.xlabel('Size of one iteration (KBytes)')
    plt.ylabel('Ratio of read:write volumes')
    plt.legend([
        'L2',
        'L3',
        'Memory',
    ])

    plt.savefig(results_file + '_ratios.png')

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
