# Memory Visualizer (C++ with SFML)

This project is a **memory visualization tool** built using **C++** and **SFML (Simple and Fast Multimedia Library)**. It provides an interactive graphical interface that simulates how memory blocks are allocated, freed, and managed in a system. The visualization includes real-time animations, statistics, and user interactions, making it a helpful tool for understanding memory operations visually.

---

## Features

* **Animated and visually appealing UI**: The layout uses glowing outlines, subtle gradients, and animated effects to make the simulation both informative and engaging.
* **Grid of memory blocks**: A 10×10 grid represents 100 memory blocks. Each block can hold a memory size from 10 KB to 100 KB.
* **Memory operations**:

  * **Allocate**: Select a block and assign a memory size.
  * **Free**: Deallocate an already allocated block.
  * **Clear All**: Reset all blocks to the initial unallocated state.
* **Live memory summary**: A side panel displays memory statistics such as:

  * Number of allocated, freed, and available blocks
  * Total memory in use
  * Minimum, maximum, and average memory size per block
  * Distribution of memory sizes across blocks
* **Interactive input**: When allocating, an input box appears where the user can enter the desired memory size.
* **Visual indicators**:

  * Allocated blocks appear in red.
  * Freed blocks appear in green.
  * Available blocks remain dark blue.
  * Selected blocks are highlighted for clarity.

---

## User Interface Overview

* **Left Panel**: Contains the control buttons (Allocate, Free, Clear All) and the application title.
* **Center Panel**: Displays the memory block grid with animations.
* **Right Panel**: Shows the memory statistics and usage distribution.

---

## Code Structure

The entire project is contained in a single file: `main.cpp`. It consists of:

* `Block` struct: Represents each memory unit and handles its appearance and animation.
* `MemoryUI` class: Manages all UI elements, rendering, and user interaction.
* `main()` function: Initializes the application window and runs the main loop.

---

## Requirements

* SFML 2.5 or later
* C++17 or later
* A TrueType font (the default is DejaVuSans; adjust the path if needed)

---

## How to Compile and Run

1. Ensure SFML is installed and configured properly.
2. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/MemoryVisualizer.git
   cd MemoryVisualizer
   ```
3. Compile with g++:

   ```bash
   g++ main.cpp -o MemoryVisualizer -lsfml-graphics -lsfml-window -lsfml-system
   ./MemoryVisualizer
   ```

---

## Possible Use Cases

* Educational tool for explaining memory management in operating systems
* Visual demo for lectures or classroom presentations
* A practical project to learn SFML and graphical interfaces in C++
* Personal experiments with UI design and animation in C++

---

## Notes

* Input for memory allocation must be between 10 KB and 100 KB.
* Blocks can only be freed if they were previously allocated.
* Memory size input is limited to three digits.
* The UI automatically updates memory statistics after each operation.

---
