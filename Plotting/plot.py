import matplotlib.pyplot as plt
import numpy as np
import re

filename = "output.txt"  # Specify the name of your input file

resultsOMP = []  # List to store the parsed resultsOMP
MPI = []
Hybrid = []


def readInOpenMP_Serial():
    # Open the file in read mode
    with open(filename, "r") as file:
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
                "openmp_time": openmp_time,
            }

            # Add the parsed data to the resultsOMP list
            resultsOMP.append(data)

        i += 3  # Increment the index to proceed to the next line


def readInMPI():
    with open("MPI.txt", "r") as file:
        lines = file.readlines()

    for line in lines:
        if line.startswith("Running MPI program with -"):
            match = re.search(r"Seed: (\d+), Size (\d+), Nodes: (\d+)", line)
            if match:
                seed = int(match.group(1))
                size = int(match.group(2))
                nodes = int(match.group(3))
            else:
                continue

        if line.startswith("MPI: Rank 0:"):
            match = re.search(r"MPI: Rank 0: (\d+\.\d+)", line)
            if match:
                time = float(match.group(1))
                MPI.append({"Seed": seed, "Size": size,
                           "Nodes": nodes, "Time": time})
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
            "Hybrid Rank 0 Time": rank_0_time,
        }

        # Append the dictionary to the data list
        Hybrid.append(info)

    # # Print the stored data
    # for info in Hybrid:
    #     print(info)
    # print(len(Hybrid))


def plotRegularSampling():

    # Get the unique sizes in the data
    # unique_sizes = set(data['size'] for data in resultsOMP)
    # unique_sizes = [12000, 120000, 1200000, 12000000, 120000000, 1200000000]
    unique_sizes = [120000, 1200000, 12000000, 120000000, 1200000000]

    # Plotting multiple lines, one for each size
    for size in unique_sizes:
        # print(size)
        # Filter the data by size
        filtered_data = [data for data in resultsOMP if data['size'] == size]

        # Extract the number of processors, serial time, and size
        processors = [data['processors'] for data in filtered_data]
        serial_time = [data['serial_time'] for data in filtered_data]

        # Calculate the speedup for each number of processors
        speedup = [serial_time[0] / data['openmp_time']
                   for data in filtered_data]

        # Plot the speedup vs. number of processors with the unique color
        plt.plot(processors, speedup, marker='o', label=f'Size = {size}')

    # Set the labels and title
    plt.xlabel('Number of Processors')
    plt.ylabel('Speedup')
    plt.title('Speedup vs. Number of Processors (Multiple Sizes)')
    plt.xscale('log', basex=2)
    plt.xticks(processors, [1, 2, 4, 8, 16, 32])
    # Add a legend
    plt.legend()

    # Display the plot
    plt.show()


def plotSerial():
    # Filter the resultsOMP for cases where processors = 1 and size = 12000, 120000, 1200000
    filtered_results = [data for data in resultsOMP if data["processors"] == 1]

    # Sort the resultsOMP based on size for proper plotting order
    filtered_results.sort(key=lambda x: x["size"])

    # Extract the size and serial time values
    sizes = [data["size"] for data in filtered_results]
    serial_times = [data["serial_time"] for data in filtered_results]

    # Plotting
    plt.plot(sizes, serial_times, marker="o")
    plt.xlabel("Size")
    plt.xscale("log")
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
    plt.plot(processors, speedup, marker="o")
    plt.xlabel("Number of Processors")
    plt.ylabel("Speedup (Serial Time / OpenMP Time)")
    plt.title("Speedup vs Number of Processors (Size = 12000)")
    plt.grid(True)
    plt.show()


def plotAllSpeedup():
    input_sizes = [120000, 1200000, 12000000, 120000000, 1200000000]

    # input_sizes = [1200000, 12000000, 120000000, 1200000000]
    markers = ['o', 's', '^', 'x', 's']  # Marker styles for each size
    colors = ['blue', 'orange', 'green']  # Colors for each size
    styles = ["--", '.', '-.',]

    # Plotting
    # plt.figure(figsize=(8, 6))
    fig, ax = plt.subplots()

    for i, size in enumerate(input_sizes):

        filtered_results = [
            data for data in resultsOMP if data["size"] == size]
        # print(filtered_results)
        # print()
        processors = [data["processors"] for data in filtered_results]

        time = filtered_results[0]["serial_time"]
        speedup = [data["serial_time"] / data["openmp_time"]
                   for data in filtered_results]
        # speedup = [time / data["openmp_time"]
        #            for data in filtered_results]

        # marker = markers[i]
        marker = ''
        # color = colors[i]

        # plt.plot(processors, speedup, marker=marker,
        #          color=color, label=f"Size = {size}")
        plt.plot(processors, speedup, marker=marker,  label=f"Size = {size}")

    plt.xlabel([1, 2, 4, 8, 16, 32])
    plt.xlabel("Number of Processors", fontsize=15)
    plt.ylabel("Speedup (Serial Time / OpenMP Time)", fontsize=15)
    plt.title("Speedup vs Number of Processors")
    plt.xscale('log', basex=2)
    plt.xticks(processors, [1, 2, 4, 8, 16, 32])

    # Set the default grid properties
    plt.grid(True, linestyle='-', linewidth=0.5)
    # a.grid(True)
    # Adjust the index as per your needs
    grid_line = ax.yaxis.get_gridlines()[8]
    # Set the linewidth of the selected grid line
    grid_line.set_linewidth(4.0)
    plt.legend()
    plt.show()


def plotMPISpeedup():
    sizes = [12000, 120000, 1200000, 12000000, 120000000, 1200000000]
    markers = ["o", "s", "^"]  # Marker styles for each size
    colors = ["blue", "orange", "green"]  # Colors for each size

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
        seed = mpi_data["Seed"]
        size = mpi_data["Size"]
        nodes = mpi_data["Nodes"]
        time = mpi_data["Time"]

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

    plt.xlabel("Number of Nodes")
    plt.ylabel("Speedup (Serial/MPI)")
    plt.title("Speedup vs Number of Nodes")
    plt.legend()
    plt.grid(True)
    plt.show()


def plotHybridSingleNodeSpeedup():
    # Create a dictionary to store data for each size
    size_data = {}

    # Combine the data from 'Hybrid' and 'resultsOMP' based on matching processors/threads
    for hybrid_data in Hybrid:
        hybrid_threads = hybrid_data["Threads"]
        hybrid_time = hybrid_data["Hybrid Rank 0 Time"]
        hybrid_nodes = hybrid_data["Nodes"]
        hybrid_size = hybrid_data["Size"]
        if hybrid_nodes == 1:
            for omp_data in resultsOMP:
                omp_processors = omp_data["processors"]
                omp_size = omp_data["size"]
                if omp_processors == hybrid_threads and omp_size == hybrid_size:
                    omp_serial_time = omp_data["serial_time"]
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
        plt.plot(threads, speedup, marker="o", label=f"Size {size}")

    plt.xlabel("Number of Threads")
    plt.ylabel("Speedup")
    plt.title("Speedup vs Number of Threads (Nodes=1)")
    plt.legend()
    plt.grid(True)
    plt.show()


def plotHybridSpeedup():
    # Create a dictionary to store data for each size
    size_data = {}

    # Combine the data from 'Hybrid' and 'resultsOMP' based on matching processors/threads
    for hybrid_data in Hybrid:
        hybrid_threads = hybrid_data["Threads"]
        hybrid_time = hybrid_data["Hybrid Rank 0 Time"]
        hybrid_nodes = hybrid_data["Nodes"]
        hybrid_size = hybrid_data["Size"]
        if hybrid_nodes == 3:
            omp_serial_times = [
                omp_data["serial_time"]
                for omp_data in resultsOMP
                if omp_data["size"] == hybrid_size
            ]
            omp_threads = [
                omp_data["processors"]
                for omp_data in resultsOMP
                if omp_data["size"] == hybrid_size
            ]
            if len(omp_serial_times) > 0:
                speedup = [
                    serial_time / hybrid_time for serial_time in omp_serial_times
                ]
                size_data[hybrid_size] = list(zip(omp_threads, speedup))

    # Plotting the speedup vs number of threads for each size
    plt.figure()
    for size, data in size_data.items():
        threads = [d[0] for d in data]
        speedup = [d[1] for d in data]
        plt.plot(threads, speedup, marker="o", label=f"Size {size}")

    for hybrid_data in Hybrid:
        hybrid_threads = hybrid_data["Threads"]
        hybrid_time = hybrid_data["Hybrid Rank 0 Time"]
        hybrid_nodes = hybrid_data["Nodes"]
        hybrid_size = hybrid_data["Size"]
        if hybrid_nodes == 4:
            omp_serial_times = [
                omp_data["serial_time"]
                for omp_data in resultsOMP
                if omp_data["size"] == hybrid_size
            ]
            omp_threads = [
                omp_data["processors"]
                for omp_data in resultsOMP
                if omp_data["size"] == hybrid_size
            ]
            if len(omp_serial_times) > 0:
                speedup = [
                    serial_time / hybrid_time for serial_time in omp_serial_times
                ]
                size_data[hybrid_size] = list(zip(omp_threads, speedup))

    # Plotting the speedup vs number of threads for each size
    # plt.figure()
    for size, data in size_data.items():
        threads = [d[0] for d in data]
        speedup = [d[1] for d in data]
        plt.plot(threads, speedup, marker="o",
                 linestyle="-.", label=f"Size {size}")

    plt.xlabel("Number of Threads")
    plt.ylabel("Speedup")
    plt.title("Speedup vs Number of Threads (Nodes=1)")
    plt.legend()
    plt.grid(True)
    plt.show()


def plotHybridBar():
    # array of subarrays. Each element in a subarray is the speedup time for a specific thread. Each element in the main array is an array holding all the speedup for that size
    speedup = {}  # size:[speedup for each thread]
    sizes = [12000, 120000, 1200000, 12000000, 120000000, 1200000000]
    node = 4

    # loop through OMP data and find all times when size = 1200
    for size in sizes:
        filtered_data = [data for data in resultsOMP if data['size'] == size]
        timesPerSizeSerial = []
        for element in filtered_data:
            timesPerSizeSerial.append(element["serial_time"])
        filtered_data = [data for data in Hybrid if (data['Size']
                         == size and data['Nodes'] == node)]
        print(filtered_data)
        print()
        timesPerSizeHybrid = []
        for element in filtered_data:
            timesPerSizeHybrid.append(element["Hybrid Rank 0 Time"])
        # print("serial", timesPerSizeSerial)
        # print("hybrid", timesPerSizeHybrid)
        speedup_array = np.array(timesPerSizeSerial) / \
            np.array(timesPerSizeHybrid)
        speedup.update({size: list(speedup_array)})

    fig, ax = plt.subplots
    fig.suptitle(f'Hybrid Speedup vs Number of Threads (Nodes={node})')

    bar_width = 0.1
    xPositions = np.arange(6)  # number of sizes we are plotting

    num_threads = [1, 2, 4, 8, 16, 32]

    for size, i in zip(speedup.values(), range(6)):
        ax.bar(xPositions+i*bar_width, size, bar_width,
               label=num_threads[i])  # 1 thread

    # ax.set_xlabel('Number of Threads')
    # ax.set_ylabel('Speedup')
    # # plt.xscale("log")
    ax.legend()
    # ax.grid(True)
    plt.show()


def test():

    speedup = {}  # size: [speedup for each thread]
    sizes = [12000, 120000, 1200000, 12000000, 120000000, 1200000000]
    nodes = [1, 2, 3, 4]

    fig, axes = plt.subplots(2, 2, figsize=(10, 8))
    fig.suptitle('Hybrid Speedup vs Size')

    # Initialize y-axis limits
    y_min = 0
    y_max = 1.2

    # Loop through nodes and create subplots
    for node, ax in zip(nodes, axes.flatten()):
        ax.set_title(f'Nodes={node}')
        ax.set_ylim(y_min, y_max)

        # Loop through sizes
        for size in sizes:
            filtered_data = [
                data for data in resultsOMP if data['size'] == size]
            timesPerSizeSerial = [element['serial_time']
                                  for element in filtered_data]

            filtered_data = [data for data in Hybrid if (
                data['Size'] == size and data['Nodes'] == node)]
            timesPerSizeHybrid = [element['Hybrid Rank 0 Time']
                                  for element in filtered_data]

            speedup_array = np.array(
                timesPerSizeSerial) / np.array(timesPerSizeHybrid)
            speedup[size] = list(speedup_array)

        print(speedup)
        print()
        bar_width = 0.1
        xPositions = np.arange(len(sizes))
        xTicks = [str(size) for size in sizes]
        num_threads = [1, 2, 4, 8, 16, 32]

        # hatching = ['//', "*", "+", "-", "o", "."]
        for size, i in zip(speedup.values(), range(6)):
            ax.bar(xPositions+i*bar_width, size, bar_width,
                   label=num_threads[i], edgecolor='black', hatch="")

        ax.set_xlabel('Size')
        ax.set_ylabel('Speedup')
        ax.set_xticks(xPositions + (len(sizes) - 1) * bar_width / 2)
        ax.set_xticklabels(xTicks)
        ax.grid(False)

        # Add a thick black horizontal line at y=1
        ax.axhline(y=1, color='black', linewidth=2)

    # Create a single legend for all subplots
    # handles, labels = ax.get_legend_handles_labels()
    # legend = fig.legend(handles, labels, loc='upper center',
    #                     ncol=len(sizes), fontsize='large')
    # legend.set_bbox_to_anchor((0.5, 0.97))  # Adjust the legend position

        # Adjust the spacing between subplots
    fig.subplots_adjust(hspace=0.3)

    # Create a single legend for all subplots
    handles, labels = ax.get_legend_handles_labels()
    legend = fig.legend(handles, labels, loc='lower center',
                        ncol=len(sizes), fontsize='large')
    # Adjust the legend position manually
    legend.set_bbox_to_anchor((0.5, 0.5))

    plt.tight_layout()  # Adjust spacing between subplots

    plt.show()


def plotOpenmpBarrierTimes():
    # Data for each barrier
    barrier_1 = [
        0.019900,
        0.227820,
        2.559414,
        0.022475,
        0.000010,
        0.000009,
        0.000015,
        0.000011,
        0.000032,
        0.000000,
        0.000033,
        0.000092,
        0.000105,
        0.000080,
        0.000115,
        0.000037,
        0.000195,
        0.000133,
    ]
    barrier_2 = [
        0.000001,
        0.000001,
        0.000002,
        0.000043,
        0.000048,
        0.000060,
        0.000108,
        0.000097,
        0.000124,
        0.000350,
        0.000221,
        0.000302,
        0.000592,
        0.000307,
        0.000562,
        0.001281,
        0.001173,
        0.001439,
    ]
    barrier_3 = [
        0.000001,
        0.000002,
        0.000003,
        0.000016,
        0.000018,
        0.000022,
        0.000071,
        0.000058,
        0.000061,
        0.000108,
        0.000059,
        0.000145,
        0.000215,
        0.000263,
        0.000254,
        0.000529,
        0.000607,
        0.000415,
    ]
    barrier_4 = [
        0.000001,
        0.000001,
        0.000000,
        0.000009,
        0.000007,
        0.000004,
        0.000015,
        0.000014,
        0.000003,
        0.000031,
        0.000037,
        0.000036,
        0.000064,
        0.000069,
        0.000063,
        0.000169,
        0.000103,
        0.000101,
    ]
    barrier_5 = [
        0.000000,
        0.000001,
        0.000000,
        0.000043,
        0.000036,
        0.000045,
        0.000080,
        0.000086,
        0.000045,
        0.000213,
        0.109117,
        0.000207,
        0.013687,
        0.141128,
        0.000337,
        0.012834,
        0.075760,
        0.000692,
    ]
    barrier_6 = [
        0.017183,
        0.190205,
        2.231373,
        0.013671,
        0.116472,
        1.321842,
        0.014802,
        0.165775,
        1.881083,
        0.016188,
        0.074105,
        2.145754,
        0.001762,
        0.033515,
        2.029498,
        0.004098,
        0.094043,
        1.980095,
    ]

    # X-axis labels
    x_labels = [
        "120000-1",
        "1200000-1",
        "12000000-1",
        "120000-2",
        "1200000-2",
        "12000000-2",
        "120000-4",
        "1200000-4",
        "12000000-4",
        "120000-8",
        "1200000-8",
        "12000000-8",
        "120000-16",
        "1200000-16",
        "12000000-16",
        "120000-32",
        "1200000-32",
        "12000000-32",
    ]

    # Bar positions on X-axis
    x_pos = np.arange(len(x_labels))
    bar_width = 0.4

    # Plotting the bars
    plt.bar(x_pos, barrier_1, bar_width, label="Barrier 1")
    # plt.bar(x_pos + bar_width, barrier_2, bar_width, label="Barrier 2")
    # plt.bar(x_pos + 2 * bar_width, barrier_3, bar_width, label="Barrier 3")
    # plt.bar(x_pos + 3 * bar_width, barrier_4, bar_width, label="Barrier 4")
    # plt.bar(x_pos + 4 * bar_width, barrier_5, bar_width, label="Barrier 5")
    plt.bar(x_pos + 1 * bar_width, barrier_6, bar_width, label="Barrier 6")

    # Setting the x-axis labels and rotating them for better visibility
    plt.xticks(x_pos, x_labels, rotation="vertical")

    # Adding a legend
    plt.legend()

    # Adding labels and title
    plt.xlabel("Size-Processors")
    plt.ylabel("Time (s)")
    plt.title("Barrier Times")

    # Displaying the graph
    plt.show()


# readInOpenMP_Serial()
# readInMPI()
# readInHybrid()
# plotSerial()
# plotOpenMPSpeedup()
# plotAllSpeedup()
# plotMPISpeedup()
# plotHybridSpeedup()
# plotHybridBar()

plotOpenmpBarrierTimes()

# plotRegularSampling()
test()
