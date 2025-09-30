/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    int context = 20;
    int instant = 1;
    int activityTime = 40;
    int currentTime = 0;

    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if (activity == "null" || duration_intr == -1)
            continue;

        // ---------------- CPU Burst ----------------
        if (activity == "CPU") {
            execution += std::to_string(currentTime) + ", " + std::to_string(duration_intr) + ", CPU burst\n";
            currentTime += duration_intr;
        }

        // ---------------- SYSCALL ----------------
        else if (activity == "SYSCALL") {
            // Standard interrupt steps (kernel switch, context save, etc.)
            auto [boilerplate, time_after_intr] = intr_boilerplate(currentTime, duration_intr, context, vectors);
            execution += boilerplate;
            currentTime = time_after_intr;

            // ISR body execution
            execution += std::to_string(currentTime) + ", " + std::to_string(activityTime)
                      + ", execute ISR body for device " + std::to_string(duration_intr) + "\n";
            currentTime += activityTime;

            // Simulate I/O device operation using device delay
            int isr_delay = 0;
            if (duration_intr < (int)delays.size()) {
                isr_delay = delays[duration_intr];
            } else {
                std::cerr << "Warning: Device " << duration_intr << " not found in device_table.txt, using 100ms default.\n";
                isr_delay = 100;
            }

            execution += std::to_string(currentTime) + ", " + std::to_string(isr_delay)
                      + ", device " + std::to_string(duration_intr) + " I/O operation\n";
            currentTime += isr_delay;

            // IRET (return from interrupt)
            execution += std::to_string(currentTime) + ", 1, IRET\n";
            currentTime += 1;
        }

        // ---------------- END_IO ----------------
        else if (activity == "END_IO") {
            // End of interrupt triggered by I/O completion
            auto [boilerplate, time_after_intr] = intr_boilerplate(currentTime, duration_intr, context, vectors);
            execution += boilerplate;
            currentTime = time_after_intr;

            // ISR for end of interrupt
            execution += std::to_string(currentTime) + ", " + std::to_string(activityTime)
                      + ", handle END_IO for device " + std::to_string(duration_intr) + "\n";
            currentTime += activityTime;

            // Return from interrupt
            execution += std::to_string(currentTime) + ", 1, IRET\n";
            currentTime += 1;
        }

        // ---------------- Unknown Activity ----------------
        else {
            std::cerr << "Warning: Unknown activity type: " << activity << std::endl;
        }
        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
