#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>
using namespace std;

//function to format time_stamp
string formatTimestamp(time_t timestamp) {
    tm* tm_ptr =  localtime(&timestamp);
    ostringstream oss;
    oss <<  put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// ===== TreeNode class for version management =====
class TreeNode {
public:
    int version_id;
    string content;
    string message;
    time_t created_timestamp;
    time_t snapshot_timestamp;
    TreeNode* parent;
    vector<TreeNode*> children;
    bool is_snapshot;

    TreeNode(int id, string content = "", TreeNode* parent = nullptr){
        this->version_id = id;
        this->content = content;
        this->parent = parent;
        this->message = "";
        this->snapshot_timestamp = 0;
        this->created_timestamp = time(nullptr);
        this->children.clear();
        this->is_snapshot = false;
    }
    void addChild(TreeNode* child){
        this->children.push_back(child);
    }
    bool isSnapshot() const{
        return  (snapshot_timestamp != 0);
    }
    void makeSnapshot(const string& msg){
        this->message = msg;
        this->snapshot_timestamp = time(nullptr);
    }
};

// ===== HashMap ==============
template<typename K, typename V>
class HashMap {
private:
    struct Node {
        K key;
        V value;
        Node* next;
        Node(K k, V v) : key(k), value(v), next(nullptr) {}
    };
    
    vector<Node*> table;
    int capacity;
    int size;
    
    // hash functions
    int hash(int key) {
        return key % capacity;
    }

    int hash(const string& key) {
        unsigned long hash = 0;
        for (char c : key) {
            hash = hash * 31 + c; 
        }
        return hash % capacity;
    }

public:
    HashMap(int cap = 100){
        this->capacity = cap;
        table.resize(capacity, nullptr); 
        size = 0;
    }
    ~HashMap(){
        for (size_t i = 0; i < table.size(); i++) {
            Node* current = table[i];
            while (current != nullptr) {
                Node* to_delete = current;
                current = current->next;
                delete to_delete;
            }
        }
    }
    template <typename Func>
    void forEach(Func f) {
        for (size_t i = 0; i < table.size(); i++) {
            Node* current = table[i];
            while (current != nullptr) {
                f(current->key, current->value);
                current = current->next;
            }
        }
    }
  
    void insert(K key, V value){
        int index = hash(key);              // Compute the bucket index using the hash function
        Node* current = table[index];       // Get the head of the linked list at that bucket

        // Traverse the linked list in this bucket to see if key already exists
        while(current != nullptr){
            if(current->key == key){
                //key exists,update the value
                current->value = value;
                return;
            }
            current = current->next;         //Go to next node in chain
        }
        // Key not found, insert new node at the head of the list for simplicity
        Node* newNode = new Node(key,value);
        newNode->next = table[index];
        table[index] = newNode;
        
        size++;
    }
    V* find(K key){
        int index = hash(key);                // Compute the bucket index using the hash function
        Node* current = table[index];         // Get the head of the linked list at that bucket
        
        while(current != nullptr){
            if(current->key == key){          
                return &(current->value);     //Return ptr to value if found
            }
            current = current->next;          //Go to next node in chain
        }
        return nullptr;                       //key not found
    }
    bool remove(K key){
        int index = hash(key);                // Compute the bucket index using the hash function
        Node* current = table[index];         // Get the head of the linked list at that bucket
        Node* prev = nullptr;

        while(current != nullptr){
            if(current->key == key){
                if(prev == nullptr){          //Node to remove is head of list
                    table[index] = current->next;
                }else{
                    //by-pass the current node
                    prev->next = current->next;
                }
                delete current;               //Free memory
                size--;                       //Update size
                return true;                  //Removal successful
            }
            prev = current;
            current = current->next;          //move to next node
        }
        throw out_of_range("key not found");      //key not found,nothing removed
    }
    int getSize() const{
        return size;
    }
};

// ==== File class =========
class File {
public:
    string filename;
    TreeNode* root;
    TreeNode* active_version;
    HashMap<int, TreeNode*> version_map;
    int total_versions;
    time_t last_modified;

    File(const string& name){
        this->filename = name;
    }
    ~File(){
        deleteSubtree(root);
    }
    void deleteSubtree(TreeNode* node) {
        if (node == nullptr) return;

        for (auto child : node->children) {
            deleteSubtree(child);
        }
        delete node;
    }
    
    void insert(const string& content){
        if(active_version->isSnapshot()){
            //Create new version
            string new_content = active_version->content+ " " +content;
            TreeNode* new_version = createNewVersion(new_content);

            //Parent - child relationship
            new_version->parent = active_version;
            active_version->children.push_back(new_version);

            //Set new_version as active version
            active_version = new_version;

            //Update version map
            version_map.insert(new_version->version_id,new_version);

            //Update last modified time
            updateLastModified();
        }
        else{
            //Modify active_version in place
            active_version->content += " " + content;
            
            //Update last modified time
            updateLastModified();
        }
    }
    void update(const string& content){
        if(active_version->isSnapshot()){
            //Create new version with replaced content
            TreeNode* new_version = createNewVersion(content);

            //Parent - child relationship
            new_version->parent = active_version;
            active_version->children.push_back(new_version);

            //Set new_version as active version
            active_version = new_version;

            //Update version map
            version_map.insert(new_version->version_id,new_version);

            //Update last modified time
            updateLastModified();
        }
        else{
            //Modify active_version in place
            active_version->content = content;
            
            //Update last modified time
            updateLastModified();
        }        
    }
    void snapshot(const string& message){
        if(!active_version->isSnapshot()){
            active_version->message = message;
            active_version->snapshot_timestamp = time(nullptr);
            active_version->is_snapshot = true;
        }else{
            throw logic_error("This version is already snapshotted");
        }
    }
    bool rollback(int version_id = -1){
        if(version_id == -1){
            //No version_id is provided
            if(active_version->parent != nullptr){
                active_version = active_version->parent;
                return true;
            }else{
                //already at Root; Cann't rollback any further
                throw logic_error("Cannot rollback , already at Root");
            }
        }else{
            // Find the version by ID using the version_map (HashMap)
            TreeNode** target = version_map.find(version_id);

            if(target != nullptr){
                active_version = *target;
                return true;
            }else{
                return false;      //Version_ID not found
            }
        }
    }
    string read() const{
        if(active_version != nullptr){
            return active_version->content;
        }else{
            throw runtime_error("No active version available");
        }
    }
    vector<string> getHistory() const{
        vector<string> history;
        TreeNode* current = active_version;
    
        //Traverse up to root
        vector<TreeNode*> path;
        while(current != nullptr){
            path.push_back(current);
            current = current->parent;
        }

        //Reverse path for chronological order
        reverse(path.begin(),path.end());

        //Collect snapshot version info
        for(auto node : path){
            if(node->isSnapshot()){
                string entry = "ID: "+ to_string(node->version_id) +", Timestamp: "+ formatTimestamp(node->snapshot_timestamp) +", Message: "+ node->message;
                history.push_back(entry);
            }
        }
        return history;
    }
    
private:
    TreeNode* createNewVersion(const string& content = ""){
        //Initialisations
        int new_version_id = total_versions;
        TreeNode* new_version = new TreeNode(new_version_id,content,nullptr);
        new_version->message = "";                      //Not snapshot yet
        new_version->created_timestamp = time(nullptr);
        new_version->snapshot_timestamp = 0;            //0 indicates not snapshot yet
        new_version->children.clear();

        total_versions++;

        return new_version;
    }
    void updateLastModified(){
        last_modified = time(nullptr);          //Set last_modified to current system time 
    }
};

// Heap for system wide analytics using template
template<typename T>
class Heap {
private:
    vector<T> data;
    bool isMaxHeap;
    
    void heapifyUp(int index){
        while(index > 0){
            int parent = (index-1)/2;

            //For maxHeap if current is greater than parent then swap;else break
            if(compare(data[index],data[parent])){
                swap(data[index],data[parent]);
                index = parent;
            }else{
                break;
            }
        }
    }
    void heapifyDown(int index){
        int size = data.size();
        while(true){
            int left = 2*index+1;
            int right = 2*index+2;
            int target = index;

            //For maxHeap: find largest among index,left,right
            if(left<size && compare(data[left],data[target])){
                target = left; 
            }
            if(right<size && compare(data[right],data[target])){
                target = right; 
            }
            if(target != index){
                swap(data[index],data[target]);
                index = target;
            }else{
                break;
            }
        }
    }
    bool compare(const T& a, const T& b){
        if(isMaxHeap){
            return a > b;                 // For max heap: parent should be greater than child
        }else{
            return a < b;                 // For min heap: parent should be less than child
        }
    }

public:
    Heap(bool maxHeap = true){
        this->isMaxHeap = maxHeap;          // Set heap type: true for max-heap, false for min-heap
    }
    void insert(const T& item){
        data.push_back(item);
        heapifyUp(data.size()-1);
    }
    T extractTop(){                         //Returns the top element and remove it         
        if (data.empty()) {
            throw  out_of_range("Heap is empty");
        }
        
        T top = data[0];
        data[0] = data.back();
        data.pop_back();
        if(!data.empty()){
            heapifyDown(0);
        }
        return top;   
    }
    T peek() const{                          //Returns the top element without removing it
        if (data.empty()) {
            throw  out_of_range("Heap is empty");
        }
        return data[0];
    }
    bool isEmpty() const{
        return (data.size() == 0);
    }
    int size() const{
        return data.size();
    }
};

// =====  File System Manager  ========
class FileSystemManager {
private:
    HashMap<string, File*> files;
    Heap<pair<time_t, string>> recentFilesHeap;  // For RECENT FILES
    Heap<pair<int, string>> biggestTreesHeap;    // For BIGGEST TREES
public:
    FileSystemManager(){
        files = HashMap<string, File*>();
        
        // Initialize heaps as max heaps
        recentFilesHeap = Heap<pair<time_t, string>>(true); 
        biggestTreesHeap = Heap<pair<int, string>>(true);
    }
    ~FileSystemManager(){
        files.forEach([](const string& key,File* fileptr){
            delete fileptr;             // delete the pointed-to File object to free memory
        });
    }
    
    bool createFile(const string& filename){
        //Chcek if file already exists
        File* file = getFile(filename);
        if (file != nullptr) {
            //File exists,Don't create duplicate
            return false;
        }else{
            //Create new File object dynamically 
            File* new_file = new File(filename);

             // Initialize the root version in the File object
            new_file->root = new TreeNode(0);
            new_file->root->message = "Initial Snapshot";
            new_file->root->created_timestamp = time(nullptr);
            new_file->root->snapshot_timestamp = new_file->root->created_timestamp;
            new_file->root->children.clear();
            new_file->active_version = new_file->root;
            new_file->total_versions = 1;
            new_file->version_map.insert(0, new_file->root);

            //insert new_file in HashMap files 
            files.insert(filename, new_file);

            updateMetrics(filename);      // Update heaps after modification
            return true;              //File created;
        }
    }
    bool readFile(const string& filename, string& result){
        File* file = getFile(filename);
        if (file != nullptr) {
            result = file->read();
            return true;
        }else{
            //File not found
            return false;
        }
    }
    bool insertFile(const string& filename, const string& content){
        File* file = getFile(filename);
        if (file != nullptr) {
            file->insert(content);
            updateMetrics(filename);   // Update heaps after modification
            return true;
        }else{
            //File not found
            return false;
        }
    }
    bool updateFile(const string& filename, const string& content){
        File* file = getFile(filename);
        if (file != nullptr) {
            file->update(content);
            updateMetrics(filename);             // Update heaps after modification
            return true;
        }else{
            //File not found
            return false;
        }
    }
    bool snapshotFile(const string& filename, const string& message){
        File* file = getFile(filename);
        if (file != nullptr) {
            file->snapshot(message);
            updateMetrics(filename);             // Update heaps after modification
            return true;
        }else{
            //File not found
            return false;
        }
    }
    bool rollbackFile(const string& filename, int version_id = -1){
        File* file = getFile(filename);
        if (file != nullptr) {
            file->rollback(version_id);
            updateMetrics(filename);              // Update heaps after modification
            return true;
        }else{
            return false;
        }
    }
    bool getHistory(const string& filename, vector<string>& history){
        File* file = getFile(filename);
        if (file != nullptr) {
            history = file->getHistory(); // Assign returned history to output parameter
            return true;
        }else{
            //File not found
            return false;
        }
    }
    vector<string> getRecentFiles(int num){
        vector<string> recentFiles;
        
        // Create a copy of the heap to pop elements without modifying the original
        Heap<pair<time_t, string>> tempHeap = recentFilesHeap;

        int count = 0;
        while(count < num && !tempHeap.isEmpty()){
            pair<time_t, string> topEntry = tempHeap.extractTop(); //take top value of Heap and pop() it
            time_t ts = topEntry.first;
            string fname = topEntry.second;
            //Check if the top entry is still valid (matches the file’s current last_modified)
            File* file = getFile(fname);
            if(file && file->last_modified == ts){
                // Valid Entry
                // Format output string (e.g., "Filename (Last Modified: YYYY-MM-DD HH:MM:SS)")
                string formattedEntry = topEntry.second + " (Last Modified: " + formatTimestamp(topEntry.first) + ")";
                recentFiles.push_back(formattedEntry);
                count++;
            }
            // Otherwise ignore entry
        }
        return recentFiles;
    }
    vector<string> getBiggestTrees(int num){
        vector<string> biggestTrees;
        
        // Create a copy of the heap to pop elements without modifying the original
        Heap<pair<int, string>> tempHeap = biggestTreesHeap;
    
        int count = 0;
        while(count < num && !tempHeap.isEmpty()) {
            pair<int, string> topEntry = tempHeap.extractTop();
            
            int versions = topEntry.first;
            string fname = topEntry.second;
            
            //Check if the top entry is still valid (matches the file’s current last_modified)
            File* file = getFile(fname);
            if(file && file->total_versions == versions){
                // Valid Entry
                // Format output string (e.g., "Filename (Versions: X)"
                string formattedEntry = topEntry.second + " (Versions: " + to_string(topEntry.first) + ")";
                biggestTrees.push_back(formattedEntry);
                count++;
            }
            //(file has newer version count), skip and keep popping
        }
        return biggestTrees;
    }
    
private:
    // Helper to get a pointer to the File object by filename, returns nullptr if not found
    File* getFile(const string& filename) {
        File** filePtr = files.find(filename);
        if (filePtr != nullptr ) {
            return *filePtr;
        }
        return nullptr;
    }
    
    // Helper to update system-wide analytics metrics related to the file
    void updateMetrics(const string& filename){
        File* file = getFile(filename);
        if (file != nullptr) {
            // Update recent files heap with last modified time and filename
            recentFilesHeap.insert({file->last_modified, filename});

            // Update biggest trees heap with total versions and filename
            biggestTreesHeap.insert({file->total_versions, filename});
        }
    }
};

// ==================== Command processor ================================
class CommandProcessor {
private:
    FileSystemManager fsManager;
    
public:

    void processCommand(const string& command){
        vector<string> tokens = parseCommand(command);
        if (!tokens.empty()) {
            executeCommand(tokens);
        }
    }
    void run(){
        string line;
        cout << "******Time-Travelling File System initialized******\n";
        cout << "Enter commands: " ;
        cout<< "(Type EXIT to quit.)\n";

        while (true) {
            cout << "> ";
            if (!getline(cin, line)) break;  // Exit on EOF or error
            if (line == "EXIT"||line=="exit"||line=="Exit") break;
            try {
                processCommand(line);
            } catch (const  exception& e) {
                cout << "Error: " << e.what() << endl;
            }
        }
    }
    
private:
    
    vector<string> parseCommand(const string& command) {
        vector<string> tokens;
        stringstream ss(command);
        string token;
        while (ss >> token) {
            if (!token.empty())
                tokens.push_back(token);
        }
        return tokens;
    }
    void executeCommand(const vector<string>& tokens){
        if (tokens.empty()) return;
        
        string cmd = tokens[0];
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        
        if (cmd == "CREATE") {
            if (tokens.size() < 2) {
                cout << "Usage: CREATE <filename>" << endl;
                return;
            }
            if (fsManager.createFile(tokens[1])) {
                cout << "File '" << tokens[1] << "' created successfully." << endl;
            } else {
                cout << "Failed to create file '" << tokens[1] << "'." << endl;
            }
        }
        else if (cmd == "READ") {
            if (tokens.size() < 2) {
                cout << "Usage: READ <filename>" << endl;
                return;
            }
            string content;
            if (fsManager.readFile(tokens[1], content)) {
                cout << content << endl;
            }
            else{
                cout << "File not found: " << tokens[1] << endl;
            }
        }
        else if (cmd == "INSERT" ) {
            if (tokens.size() < 3) {
                cout << "Usage: INSERT <filename> <content>" << endl;
                return;
            }
            string content = tokens[2];
            for(size_t i = 3;i<tokens.size();i++){
                content += " " + tokens[i];
            }
            if (fsManager.insertFile(tokens[1], content)) {
                cout << "Content inserted successfully." << endl;
            } else {
                cout << "Failed to insert content." << endl;
            }
        }
        else if (cmd == "UPDATE") {
            if (tokens.size() < 3) {
                cout << "Usage: UPDATE <filename> <content>" << endl;
                return;
            }
            string content = tokens[2];
            for(size_t i = 3;i<tokens.size();i++){
                content += " " + tokens[i];
            }
            if (fsManager.updateFile(tokens[1], content)) {
                cout << "Content updated successfully." << endl;
            } else {
                cout << "Failed to update content." << endl;
            }
        }
        else if (cmd == "SNAPSHOT") {
            if (tokens.size() < 3) {
                cout << "Usage: SNAPSHOT <filename> <message>" << endl;
                return;
            }
            string message = tokens[2];
            for(size_t i = 3;i<tokens.size();i++){
                message += " " + tokens[i];
            }
            if (fsManager.snapshotFile(tokens[1], message)) {
                cout << "Snapshot created successfully." << endl;
            } else {
                cout << "Failed to create snapshot." << endl;
            }
        }
        else if (cmd == "ROLLBACK") {
            if (tokens.size() < 2) {
                cout << "Usage: ROLLBACK <filename> [version_id]" << endl;
                return;
            }
            int version_id = -1;
            if (tokens.size() >= 3) {
                version_id = stoi(tokens[2]);
            }
            if (fsManager.rollbackFile(tokens[1], version_id)) {
                cout << "Rollback successful." << endl;
            } else {
                cout << "Rollback failed." << endl;
            }
            
        }
        else if (cmd == "HISTORY") {
            if (tokens.size() < 2) {
                cout << "Usage: HISTORY <filename>" << endl;
                return;
            }
            vector<string> history;
            if (fsManager.getHistory(tokens[1], history)) {
                for(string& str : history){
                    cout << str << endl;
                }
            }else {
                cout << "Failed to get history for " << tokens[1] << endl;
            }
        }
        else if (cmd == "RECENT_FILES") {
            if (tokens.size() < 2 ) {
                cout << "Usage: RECENT_FILES [num]" << endl;
                return;
            }
            int num = 10; // default
            if (tokens.size() >= 2) {
                num = stoi(tokens[1]);
            }
            vector<string> recent = fsManager.getRecentFiles(num);
            for (const string& filename : recent) {
                cout << filename << endl;
            }
        }
        else if (cmd == "BIGGEST_TREES") {
            if (tokens.size() < 2) {
                cout << "Usage: BIGGEST TREES [num]" << endl;
                return;
            }
            int num = 10; // default
            if (tokens.size() >= 2) {
                num = stoi(tokens[1]);
            }
            vector<string> biggest = fsManager.getBiggestTrees(num);
            for (const string& filename : biggest) {
                cout << filename << endl;
            }
        }
        else {
            cout << "Unknown command: " << cmd << endl;
            cout << "Available commands: CREATE, READ, INSERT, UPDATE, SNAPSHOT, ROLLBACK, HISTORY, RECENT_FILES, BIGGEST_TREES, EXIT" << endl;
        }
    }
};

#endif