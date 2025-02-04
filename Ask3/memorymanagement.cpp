#include <iostream>
#include <queue>
#include <vector>
#include <iomanip>
#include <limits>

using namespace std;

struct Process {
    int pid;
    int arrival_time;
    int duration;
    int remaining_time;
    int memory_needed;
    bool in_memory;
};

struct MemoryBlock {
    int start;
    int size;
    bool free;
    int pid; // -1 if free

    MemoryBlock(int s, int sz, bool f, int p) : start(s), size(sz), free(f), pid(p) {}
};

const int TOTAL_MEMORY = 512;  // Total memory in KB
const int TIME_QUANTUM = 3;    // Time quantum for Round Robin

vector<MemoryBlock> memory;    // Memory blocks
queue<int> ready_queue;        // Queue contains indices of processes
vector<Process> processes;

void log_state(int current_time, const Process* running_process) {
    cout << "Time: " << current_time << endl;

    if (running_process) {
        cout << "CPU: Process " << running_process->pid << " (remaining time: "
             << running_process->remaining_time << " ms)" << endl;
    } else {
        cout << "CPU: Idle" << endl;
    }

    cout << "Memory State:" << endl;
    for (size_t i = 0; i < memory.size(); ++i) {
        MemoryBlock& block = memory[i];
        cout << "[" << block.start << ", " << block.start + block.size - 1 << "] "
             << (block.free ? "Free" : "Allocated to Process " + to_string(block.pid)) << endl;
    }
    cout << "-----------------------------------" << endl;
}

bool allocate_memory(Process& process) {
    for (size_t i = 0; i < memory.size(); ++i) {
        MemoryBlock& block = memory[i];
        if (block.free && block.size >= process.memory_needed) {
            block.free = false;
            block.pid = process.pid;

            if (block.size > process.memory_needed) {
                memory.emplace(memory.begin() + (i + 1),
                               MemoryBlock(block.start + process.memory_needed, block.size - process.memory_needed, true, -1));
                block.size = process.memory_needed;
            }

            process.in_memory = true;
            return true;
        }
    }
    return false;
}

void free_memory(int pid) {
    for (size_t i = 0; i < memory.size(); ++i) {
        MemoryBlock& block = memory[i];
        if (block.pid == pid) {
            block.free = true;
            block.pid = -1;
        }
    }

    for (size_t i = 0; i < memory.size() - 1; ++i) {
        if (memory[i].free && memory[i + 1].free) {
            memory[i].size += memory[i + 1].size;
            memory.erase(memory.begin() + i + 1);
            --i;
        }
    }
}

void simulate() {
    int current_time = 0;
    int finished_processes = 0;
    size_t total_processes = processes.size();

    while (finished_processes < total_processes) {
        for (size_t i = 0; i < processes.size(); ++i) {
            Process& process = processes[i];
            if (process.arrival_time <= current_time && !process.in_memory) {
                if (allocate_memory(process)) {
                    ready_queue.push(i);
                }
            }
        }

        Process* running_process = nullptr;
        if (!ready_queue.empty()) {
            int process_idx = ready_queue.front();
            ready_queue.pop();

            running_process = &processes[process_idx];
            int execution_time = min(TIME_QUANTUM, running_process->remaining_time);
            running_process->remaining_time -= execution_time;
            current_time += execution_time;

            if (running_process->remaining_time == 0) {
                free_memory(running_process->pid);
                finished_processes++;
            } else {
                ready_queue.push(process_idx);
            }
        } else {
            current_time++;
        }

        log_state(current_time, running_process);
    }
}

int main() {
    memory.push_back(MemoryBlock(0, TOTAL_MEMORY, true, -1));

    int n = 5; // Number of processes
    cout << "Enter the number of processes (default is 5): ";
    cin >> n;
    if (!cin) {
        cout << "Invalid input. Using default value of 5.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        n = 5;
    }

    for (int i = 0; i < n; ++i) {
        Process p;
        cout << "Enter PID, Arrival Time, Duration, and Memory Needed for Process " << i + 1 << ": ";
        cin >> p.pid >> p.arrival_time >> p.duration >> p.memory_needed;

        if (!cin) {
            cout << "Invalid input detected. Aborting.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return 1;
        }

        p.remaining_time = p.duration;
        p.in_memory = false;
        processes.push_back(p);
        cout << "Process " << i + 1 << " recorded.\n"; // Debug confirmation
    }

    cout << "Starting simulation...\n";
    simulate();

    return 0;
}
