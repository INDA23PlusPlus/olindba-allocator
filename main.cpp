#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

struct BuddyNode {
    size_t size;
    bool used = false;
    size_t pos;

    BuddyNode() {}
    BuddyNode(size_t _size, size_t _pos) {
        size = _size;
        pos = _pos;
    }
    BuddyNode(size_t _size, bool _used, size_t _pos) {
        size = _size;
        used = _used;
        pos = _pos;
    }
    bool operator < (BuddyNode b) const {
        return b.pos > pos;
    }
};

class BuddyAllocator {
    void* bytes = nullptr;
    std::vector<BuddyNode> buddy_list;
    std::vector<int> used;

public:
    
    BuddyAllocator(size_t size) {
        bytes = std::malloc(size);
        buddy_list.push_back(BuddyNode(size, 0));
    }

    void* alloc(size_t size) {
        size_t largest_can_use = 0;
        for (int i = 0; i < buddy_list.size(); i++) {
            if (!buddy_list[i].used) largest_can_use = std::max(largest_can_use, buddy_list[i].size);
        }

        if (largest_can_use == 0) return nullptr;

        for (int i = 0; i < buddy_list.size(); i++) {
            if (!buddy_list[i].used && buddy_list[i].size == largest_can_use) {
                while (true) {
                    if (buddy_list[i].size / 2 >= size) {
                        buddy_list[i].size /= 2;
                        buddy_list.push_back(BuddyNode(buddy_list[i].size, buddy_list[i].pos + buddy_list[i].size));
                    }
                    else break;
                }
                buddy_list[i].used = true;
                return (void*) ((size_t)bytes + buddy_list[i].pos);
                break;
            }
        }
        return nullptr;
    }

    void free(void* ptr) {
        sort(buddy_list.begin(), buddy_list.end());
        for (int i = 0; i < buddy_list.size(); i++) {
            if ((size_t)bytes + buddy_list[i].pos == (size_t)ptr) {
                buddy_list[i].used = false;
                while (true) {
                    if (i > 0 && buddy_list[i - 1].size == buddy_list[i].size && !buddy_list[i - 1].used) {
                        buddy_list[i - 1].size *= 2;
                        buddy_list.erase(buddy_list.begin() + i);
                    }
                    else if (i < buddy_list.size() - 1 && buddy_list[i + 1].size == buddy_list[i].size && !buddy_list[i + 1].used) {
                        buddy_list[i].size *= 2;
                        buddy_list.erase(buddy_list.begin() + i + 1);
                    }
                    else {
                        break;
                    }
                }
                break;
            }
        }
    }

    void* resize(void* ptr, size_t old_size, size_t new_size) {
        free(ptr);
        void* new_ptr = alloc(new_size);
        std::memcpy(new_ptr, ptr, old_size);
        return new_ptr;
    }

    void print_used_buddies() {
        std::cout << "Used buddies:" << std::endl;
        for (int i = 0; i < buddy_list.size(); i++) if (buddy_list[i].used) {
            std::cout << "Size: " << buddy_list[i].size << ", Byte offset: " << buddy_list[i].pos << std::endl;
        }
    }

    ~BuddyAllocator() {
        std::free(bytes);
    }
};

void buddy_allocater_test() {
    BuddyAllocator allocator = BuddyAllocator(4096);

    //Integer
    std::cout << "Allocate integer memory" << std::endl;
    int* list_size = (int*)allocator.alloc(sizeof(int));
    allocator.print_used_buddies();
    *list_size = 20;
    std::cout << "Address: " << list_size << ", Value: " << *list_size << std::endl;
    std::cout << "<------------------------------------------------------------------------------->" << std::endl;

    //Integer list
    std::cout << "Allocate 20 integer list memory" << std::endl;
    int* int_list = (int*)allocator.alloc(*list_size * sizeof(int));
    allocator.print_used_buddies();
    for (int i = 0; i < *list_size; i++) {
        int_list[i] = i * i;
    }
    std::cout << "Address: " << int_list << ", Value: ";
    for (int i = 0; i < *list_size; i++) {
        std::cout << int_list[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "<------------------------------------------------------------------------------->" << std::endl;

    //Resize list
    std::cout << "Resize integer list 40" << std::endl;
    allocator.resize(int_list, *list_size * sizeof(int), 2 * *list_size * sizeof(int));
    allocator.print_used_buddies();
    for (int i = 0; i < *list_size; i++) {
        int_list[*list_size + i] = -i * i;
    }
    std::cout << "Address: " << int_list << ", Value: ";
    for (int i = 0; i < 2 * *list_size; i++) {
        std::cout << int_list[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "<------------------------------------------------------------------------------->" << std::endl;

    //String
    std::cout << "Allocate string  memory" << std::endl;
    char* string = (char*)allocator.alloc(20 * sizeof(char));
    allocator.print_used_buddies();
    strcpy(string, "lots of charachters");
    std::cout << "Address: " << (void*)&string[0] << ", Value: " << string << std::endl;
    std::cout << "<------------------------------------------------------------------------------->" << std::endl;
}

class ArenaAllocator {
    void* bytes = nullptr;
    size_t total_size = 0;
    size_t bytes_written = 0;

public:

    ArenaAllocator(size_t size) {
        bytes = std::malloc(size);
        total_size = size;
        bytes_written = 0;
    }

    void* alloc(size_t size) {
        auto value = bytes_written + (size_t)bytes;
        if (bytes_written + size > total_size) {
            return nullptr;
        }
        bytes_written += size;
        return (void*)value;
    }

    void free() {
        std::free(bytes);
    }
};

void arena_allocator_test() {
    ArenaAllocator allocator = ArenaAllocator(1024);

    //Integer
    int* list_size = (int*)allocator.alloc(sizeof(int));
    *list_size = 25;

    //Integer list
    int* int_list = (int*)allocator.alloc(*list_size * sizeof(int));
    for (int i = 0; i < *list_size; i++) {
        int_list[i] = i * i;
    }

    //String
    char* string = (char*)allocator.alloc(20 * sizeof(char));
    strcpy(string, "lots of charachters");

    //64-bit integer
    long long* int64 = (long long*)allocator.alloc(sizeof(long long));
    *int64 = 298745698762348325;

    //Addresses
    std::cout << list_size << " " << int_list << " " <<  (void*)&string[0] << " " << int64 << std::endl;

    //Values
    std::cout << *list_size << std::endl;

    for (int i = 0; i < *list_size; i++) {
        std::cout << int_list[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << string << std::endl;

    std::cout << *int64 << std::endl;

    allocator.free();    
}

int main() {
    arena_allocator_test();
    std::cout << "\n\n";
    buddy_allocater_test();
    return 0;
}