import matplotlib.pyplot as plt
import numpy as np
import re

filename = "output.txt"  # Specify the name of your input file

resultsOMP = []  # List to store the parsed resultsOMP
MPI = []
Hybrid = []


def readInOpenMP_Serial():
    # Open the file in read mode
    with open(filename, 'r') as file:
        lines = file.readlines()  # Read all lines from the file

    # Iterate over the lines and process the data
    i = 0
    while i < len(lines):
        line = lines[i].strip()  # Remove leading/trailing whitespaces

        if line.startswith("Seed"):
            # Split the line by ", " to extract seed, size, and processors
            seed, size, processors = line.split(", ")
            # Extract the seed value as an integer
            seed = int(seed.split(" ")[1])
            # Extract the size value as an integer
            size = int(size.split(" ")[1])
            # Extract the processors value as an integer
            processors = int(processors.split(" ")[1])

            # Extract the corresponding serial and OpenMP times
            serial_time = float(lines[i + 1].split(": ")[1])
            openmp_time = float(lines[i + 2].split(": ")[1])

            # Create a dictionary to store the parsed data
            data = {
                "seed": seed,
                "size": size,
                "processors": processors,
                "serial_time": serial_time,
                "openmp_time": openmp_time
            }

            # Add the parsed data to the resultsOMP list
            resultsOMP.append(data)

        i += 3  # Increment the index to proceed to the next line


def readInMPI():
    with open('MPI.txt', 'r') as file:
        lines = file.readlines()

    for line in lines:
        if line.startswith('Running MPI program with -'):
            match = re.search(r'Seed: (\d+), Size (\d+), Nodes: (\d+)', line)
            if match:
                seed = int(match.group(1))
                size = int(match.group(2))
                nodes = int(match.group(3))
            else:
                continue

        if line.startswith('MPI: Rank 0:'):
            match = re.search(r'MPI: Rank 0: (\d+\.\d+)', line)
            if match:
                time = float(match.group(1))
                MPI.append({'Seed': seed, 'Size': size,
                            'Nodes': nodes, 'Time': time})
    # for entry in MPI:
    #     print(entry)


def readInHybrid():
    file_path = "hybrid.txt"  # Replace with the path to your .txt file

    # Define regular expressions to match the required information
    pattern = r"Running MPI program with - Seed: (\d+), Size (\d+), Nodes: (\d+), Threads: (\d+)\nHybrid: Rank 0: (\d+\.\d+)"
    regex = re.compile(pattern)

    # Read the data from the file
    with open(file_path, "r") as file:
        input_text = file.read()

    # Find all matches in the input text
    matches = regex.findall(input_text)

    # Store the extracted information

    for match in matches:
        seed = int(match[0])
        size = int(match[1])
        nodes = int(match[2])
        threads = int(match[3])
        rank_0_time = float(match[4])

        # Create a dictionary to store the information
        info = {
            "Seed": seed,
            "Size": size,
            "Nodes": nodes,
            "Threads": threads,
            "Hybrid Rank 0 Time": rank_0_time
        }

        # Append the dictionary to the data list
        Hybrid.append(info)

    # # Print the stored data
    # for info in Hybrid:
    #     print(info)


def plotSerial():
    # Filter the resultsOMP for cases where processors = 1 and size = 12000, 120000, 1200000
    filtered_results = [data for data in resultsOMP if data["processors"]
                        == 1]

    # Sort the resultsOMP based on size for proper plotting order
    filtered_results.sort(key=lambda x: x["size"])

    # Extract the size and serial time values
    sizes = [data["size"] for data in filtered_results]
    serial_times = [data["serial_time"] for data in filtered_results]

    # Plotting
    plt.plot(sizes, serial_times, marker='o')
    plt.xlabel("Size")
    plt.xscale('log')
    plt.ylabel("Serial Time")
    plt.title("Serial Time vs Size (Processors = 1)")
    plt.grid(True)
    plt.show()


def plotOpenMPSpeedup():
    # Filter the resultsOMP for size = 12000 and calculate the speedup
    filtered_results = [
        data for data in resultsOMP if data["size"] == 1200000000]
    processors = [data["processors"] for data in filtered_results]
    speedup = [data["serial_time"] / data["openmp_time"]
               for data in filtered_results]

    # speedup = [data["openmp_time"]
    #            for data in filtered_results]

    # Plotting
    plt.plot(processors, speedup, marker='o')
    plt.xlabel("Number of Processors")
    plt.ylabel("Speedup (Serial Time / OpenMP Time)")
    plt.title("Speedup vs Number of Processors (Size = 12000)")
    plt.grid(True)
    plt.show()


def plotAllSpeedup():
    input_sizes = [120000, 1200000, 12000000, 120000000, 1200000000]
    markers = ['o', 's', '^', 'x', '']  # Marker styles for each size
    colors = ['blue', 'orange', 'green']  # Colors for each size

    # Plotting
    plt.figure(figsize=(8, 6))

    for i, size in enumerate(input_sizes):
        filtered_results = [
            data for data in resultsOMP if data["size"] == size]
        processors = [data["processors"] for data in filtered_results]

        # time = filtered_results[0]["serial_time"]
        # serial_time = data[]
        # data["serial_time"]
        # speedup = [data["serial_time"] for data in filtered_results]
        speedup = [data["serial_time"] / data["openmp_time"]
                   for data in filtered_results]
        # marker = markers[i]
        # color = colors[i]

        # plt.plot(processors, speedup, marker=marker,
        #          color=color, label=f"Size = {size}")
        plt.plot(processors, speedup, label=f"Size = {size}")

    plt.xlabel("Number of Processors")
    plt.ylabel("Speedup (Serial Time / OpenMP Time)")
    plt.title("Speedup vs Number of Processors")
    # plt.yscale('log')
    plt.grid(True)
    plt.legend()
    plt.show()


def plotMPISpeedup():
    sizes = [12000, 120000, 1200000, 12000000, 120000000, 1200000000]
    markers = ['o', 's', '^']  # Marker styles for each size
    colors = ['blue', 'orange', 'green']  # Colors for each size

    # filtered_results = [data for data in MPI if data["size"] == 12000]

    # Plotting
    # serial time is the serial time of this size with processors =1
    # the MPI time is the time recorded for this size and for this node
    serial_times_for_size = []
    for data in resultsOMP:
        if data["processors"] == 1:
            # Check if the processors value is 1
            serial_time = data["serial_time"]
            size = data["size"]
            serial_times_for_size.append((size, serial_time))

    # Create empty lists to store the data for each input size
    input_sizes = []
    speedups = []

    # Iterate over the MPI data
    for mpi_data in MPI:
        seed = mpi_data['Seed']
        size = mpi_data['Size']
        nodes = mpi_data['Nodes']
        time = mpi_data['Time']

        # Find the corresponding serial_time for the current input size
        for serial_data in serial_times_for_size:
            if serial_data[0] == size:
                serial_time = serial_data[1]
                break

        # Calculate the speedup as serial_time divided by MPI time for rank 0
        speedup = serial_time / time

        # Append the input size and speedup to the respective lists
        input_sizes.append(nodes)
        speedups.append(speedup)
        # print(input_sizes)
        # print(speedups)

    # Plotting the speedup vs number of nodes
    plt.figure()
    arr1 = speedups[0::6]
    arr2 = speedups[1::6]
    arr3 = speedups[2::6]
    arr4 = speedups[3::6]
    arr5 = speedups[4::6]
    arr6 = speedups[5::6]

    plt.plot([1, 2, 3, 4], arr1, label=sizes[0])
    plt.plot([1, 2, 3, 4], arr2, label=sizes[1])
    plt.plot([1, 2, 3, 4], arr3, label=sizes[2])
    plt.plot([1, 2, 3, 4], arr4, label=sizes[3])
    plt.plot([1, 2, 3, 4], arr5, label=sizes[4])
    plt.plot([1, 2, 3, 4], arr6, label=sizes[5])

    plt.xlabel('Number of Nodes')
    plt.ylabel('Speedup (Serial/MPI)')
    plt.title('Speedup vs Number of Nodes')
    plt.legend()
    plt.grid(True)
    plt.show()


def plotHybridSingleNodeSpeedup():
    # Create a dictionary to store data for each size
    size_data = {}

    # Combine the data from 'Hybrid' and 'resultsOMP' based on matching processors/threads
    for hybrid_data in Hybrid:
        hybrid_threads = hybrid_data['Threads']
        hybrid_time = hybrid_data['Hybrid Rank 0 Time']
        hybrid_nodes = hybrid_data['Nodes']
        hybrid_size = hybrid_data['Size']
        if hybrid_nodes == 1:
            for omp_data in resultsOMP:
                omp_processors = omp_data['processors']
                omp_size = omp_data['size']
                if omp_processors == hybrid_threads and omp_size == hybrid_size:
                    omp_serial_time = omp_data['serial_time']
                    speedup = omp_serial_time / hybrid_time
                    if hybrid_size in size_data:
                        size_data[hybrid_size].append(
                            (hybrid_threads, speedup))
                    else:
                        size_data[hybrid_size] = [(hybrid_threads, speedup)]
                    break

    # Plotting the speedup vs number of threads for each size
    plt.figure()
    for size, data in size_data.items():
        threads = [d[0] for d in data]
        speedup = [d[1] for d in data]
        plt.plot(threads, speedup, marker='o', label=f"Size {size}")

    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs Number of Threads (Nodes=1)')
    plt.legend()
    plt.grid(True)
    plt.show()


def plotHybridSpeedup():
    # Create a dictionary to store data for each size
    size_data = {}

    # Combine the data from 'Hybrid' and 'resultsOMP' based on matching processors/threads
    for hybrid_data in Hybrid:
        hybrid_threads = hybrid_data['Threads']
        hybrid_time = hybrid_data['Hybrid Rank 0 Time']
        hybrid_nodes = hybrid_data['Nodes']
        hybrid_size = hybrid_data['Size']
        if hybrid_nodes == 3:
            omp_serial_times = [omp_data['serial_time']
                                for omp_data in resultsOMP if omp_data['size'] == hybrid_size]
            omp_threads = [omp_data['processors']
                           for omp_data in resultsOMP if omp_data['size'] == hybrid_size]
            if len(omp_serial_times) > 0:
                speedup = [serial_time /
                           hybrid_time for serial_time in omp_serial_times]
                size_data[hybrid_size] = list(zip(omp_threads, speedup))

    # Plotting the speedup vs number of threads for each size
    plt.figure()
    for size, data in size_data.items():
        threads = [d[0] for d in data]
        speedup = [d[1] for d in data]
        plt.plot(threads, speedup, marker='o', label=f"Size {size}")

    for hybrid_data in Hybrid:
        hybrid_threads = hybrid_data['Threads']
        hybrid_time = hybrid_data['Hybrid Rank 0 Time']
        hybrid_nodes = hybrid_data['Nodes']
        hybrid_size = hybrid_data['Size']
        if hybrid_nodes == 4:
            omp_serial_times = [omp_data['serial_time']
                                for omp_data in resultsOMP if omp_data['size'] == hybrid_size]
            omp_threads = [omp_data['processors']
                           for omp_data in resultsOMP if omp_data['size'] == hybrid_size]
            if len(omp_serial_times) > 0:
                speedup = [serial_time /
                           hybrid_time for serial_time in omp_serial_times]
                size_data[hybrid_size] = list(zip(omp_threads, speedup))

    # Plotting the speedup vs number of threads for each size
    # plt.figure()
    for size, data in size_data.items():
        threads = [d[0] for d in data]
        speedup = [d[1] for d in data]
        plt.plot(threads, speedup, marker='o',
                 linestyle="-.", label=f"Size {size}")

    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs Number of Threads (Nodes=1)')
    plt.legend()
    plt.grid(True)
    plt.show()


def plotHybridBar():
    # Create a dictionary to store data for each size
    size_data = {}

    # Combine the data from 'Hybrid' and 'resultsOMP' based on matching processors/threads
    for hybrid_data in Hybrid:
        hybrid_threads = hybrid_data['Threads']
        hybrid_time = hybrid_data['Hybrid Rank 0 Time']
        hybrid_nodes = hybrid_data['Nodes']
        hybrid_size = hybrid_data['Size']
        if hybrid_nodes == 1:
            omp_serial_times = [omp_data['serial_time']
                                for omp_data in resultsOMP if omp_data['size'] == hybrid_size]
            omp_threads = [omp_data['processors']
                           for omp_data in resultsOMP if omp_data['size'] == hybrid_size]
            if len(omp_serial_times) > 0:
                speedup = [serial_time /
                           hybrid_time for serial_time in omp_serial_times]
                size_data[hybrid_size] = list(zip(omp_threads, speedup))

    # Plotting the speedup vs number of threads for each size (Nodes=1)
    fig, ax = plt.subplots()
    fig.suptitle('Speedup vs Number of Threads (Nodes=1)')

    for size, data in size_data.items():
        threads = [d[0] for d in data]
        speedup = [d[1] for d in data]
        ax.bar(threads, speedup, label=f"Size {size}")

    ax.set_xlabel('Number of Threads')
    ax.set_ylabel('Speedup')
    plt.xscale("log")
    ax.legend()
    ax.grid(True)
    plt.show()


readInOpenMP_Serial()
readInMPI()
readInHybrid()
# plotSerial()
# plotOpenMPSpeedup()
# plotAllSpeedup()
# plotMPISpeedup()
# plotHybridSpeedup()
plotHybridBar()
