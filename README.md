============================ Time-Travelling File System ==========================================================
Author 	  --> Saurabh Pandey
Mail      --> ee3240043@iitd.ac.in
Course    --> COL106

1. Project Overview:
	This project implements a simplified, in-memory version control system inspired by Git.
	The system is built from scratch using Trees, HashMaps, and Heaps, without relying on C++ STL implementations of these structures.
2. Features:
	It supports file creation, content modification, snapshots, rollbacks, and system-wide analytics (recent files, biggest version trees).
3. Project Structure:
	main.cpp              # Contains main() entry point
	file_system.hpp       # Core implementation (TreeNode, File, HashMap, Heap, FileSystemManager, CommandProcessor)
	run.bat		      # Batch script to compile and run the program on Windows 
	compile.sh	      # Shell script to compile and run the program 
	ReadMe.txt            # Documentation (this file)
4. Compilation Instructions:
	4.1. Windows 
		1. Open Command Prompt or PowerShell.
		2. Navigate to the project folder:
				cd path\to\project
		3. Type in terminal:
				.\run.bat
	4.2. Linux/macOS
		1. Open Terminal.
 		2. Navigate to the project folder:
				cd path/to/project
		3. Make the shell script executable (only once):
				chmod +x compile.sh
		4. Run the script:
				./compile.sh
	This will compile and run the program automatically.
	If compilation succeeds, it will produce an executable named file_system and run it.
	If compilation fails, an error message will be displayed.
5. Running the Program:
	When executed, the program initializes the Time-Travelling File System and waits for user input:
	Example startup message:

		******Time-Travelling File System initialized.******
		Enter commands: (Type EXIT to quit.)
		>

	Enter commands via stdin (terminal input). 
	The program continues until you type:
		EXIT  (or exit or Exit)
6. Command Reference: 
	6.1. Core File Operations :
		1. CREATE <filename>
		   Creates a new file with root version (ID 0) and an initial snapshot.
		2. READ <filename>
		   Prints the content of the file’s active version.
		3. INSERT <filename> <content>
		   Appends content to the active version. Creates a new version if the active version is already a snapshot.
		4. UPDATE <filename> <content>
	           Replaces the file’s content (new version if current is snapshot, otherwise modifies in place).
		5. SNAPSHOT <filename> <message>
	           Marks the active version as immutable with a snapshot message and timestamp.
		6. ROLLBACK <filename> [versionID]
	           Restores the active version pointer. 
		    With versionID: rolls back to that version.
      		    Without versionID: rolls back to the parent of the current version.(if exists)
		7. HISTORY <filename>
		   Lists all snapshotted versions of the file chronologically with ID, timestamp, and message , which lie on the path
	           from active node to the root in the file tree
	6.2 System-Wide Analytics:
		1. RECENT_FILES [num]
       		   Lists up to [num] files ordered by last modification time.
		2. BIGGEST_TREES [num]
		   Lists up to [num] files ordered by their total version count.
	**NOTE** All operations are Case insensitive meaning { Create <file> == create <file> == CREATE <file> }

7. Error Handling:
	1. Creating a file that already exists → "Failed to create file"
	2. Reading, inserting, updating, snapshotting, or rolling back a non-existent file → "File not found"
	3. Taking a snapshot on an already snapshotted version → "This version is already snapshotted"
	4. Rolling back beyond the root → "Cannot rollback, already at Root"
	5. Removing non-existent keys from HashMap → throws out_of_range("key not found")
	6. Entering an unknown command →
		Unknown command: <your_input>
		Available commands: CREATE, READ, INSERT, UPDATE, SNAPSHOT, ROLLBACK, HISTORY, RECENT_FILES, BIGGEST_TREES, EXIT
	7. Supplying too few arguments for certain commands → prints usage help. Examples:
		RECENT_FILES without number → "Usage: RECENT_FILES [num]"
		BIGGEST_TREES without number → "Usage: BIGGEST TREES [num]"
		CREATE, READ, INSERT, UPDATE, SNAPSHOT, ROLLBACK, HISTORY with missing filename or content → usage message shown accordingly.
8. Example:

	> CREATE notes
	File 'notes' created successfully.
	> INSERT notes Hello
	Content inserted successfully.
	> SNAPSHOT notes First version
	Snapshot created successfully.
	> INSERT notes World
	Content inserted successfully.
	> READ notes
	HelloWorld
	> HISTORY notes
	ID: 0, Timestamp: 2025-09-10 15:20:45, Message: Initial Snapshot
	ID: 1, Timestamp: 2025-09-10 15:21:10, Message: First version
	> RECENT_FILES 5
	notes (Last Modified: 2025-09-10 15:21:11)
	> BIGGEST_TREES 5
	notes (Versions: 3)
	> EXIT
8. Conclusion:

	The Time-Travelling File System demonstrates how core data structures such as trees, hash maps, and heaps can be combined to build a simplified 		version control system from scratch.
	It provides essential operations like file creation, modification, snapshots, rollbacks, and system-wide analytics, all without relying on built-in 		C++ STL implementations of these structures.

	Future Improvements:

		1. Add persistent storage (currently in-memory only).
		2. Support for directories and nested file structures.
		3. More advanced analytics (e.g., most frequently edited files).

		4. Enhanced user interface for easier command interaction.
