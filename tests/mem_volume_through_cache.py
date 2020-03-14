import csv
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

    while data_size_kb < 1000001:
        print('running memory with data size ' + str(data_size_kb),
              end='', flush=True)
        start_time = time.time()
        child = subprocess.run(
            [fhv_bin_location, '--benchmark-cache-and-memory', 
             str(int(start_num_iterations/data_size_kb)), str(data_size_kb),
             '-c'], stdout=subprocess.PIPE)
        elapsed_time = time.time() - start_time
        print(' {:.3f}s'.format(elapsed_time))
        output.append(child.stdout.decode().strip().split(','))
        data_size_kb *= 10

    write_csv(results_file + '.csv', get_header(), output)

def main():
    run_tests()

if __name__ == '__main__':
    main()
