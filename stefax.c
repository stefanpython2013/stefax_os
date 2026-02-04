// ========== GLOBAL VARIABLES ==========
unsigned int current_line = 0;
#define MAX_ROWS 25
#define MAX_COLS 80

// ========== VIDEO FUNCTIONS ==========
void scroll_screen() {
    char *vidptr = (char*)0xb8000;
    
    // Move all lines up by one
    for(int i = 0; i < (MAX_ROWS - 1) * MAX_COLS * 2; i++) {
        vidptr[i] = vidptr[i + MAX_COLS * 2];
    }
    
    // Clear the last line
    for(int i = (MAX_ROWS - 1) * MAX_COLS * 2; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        vidptr[i] = ' ';
        vidptr[i + 1] = 0x07;
    }
    
    current_line--;
}

void check_scroll() {
    if(current_line >= MAX_ROWS) {
        scroll_screen();
    }
}

void print(char *str) {
    char *vidptr = (char*)0xb8000;
    unsigned int i = 0;
    unsigned int j = 0;
    
    // Clear screen - fill with spaces
    while(j < 80 * 25 * 2) {
        vidptr[j] = ' ';
        vidptr[j + 1] = 0x07;
        j = j + 2;
    }
    
    j = 0;
    i = 0;
    
    // Write string to video memory
    while(str[j] != '\0') {
        vidptr[i] = str[j];
        vidptr[i + 1] = 0x07;
        i = i + 2;
        j++;
    }
}

void print_at(char *str, unsigned int pos) {
    char *vidptr = (char*)0xb8000;
    unsigned int i = pos * 2;
    unsigned int j = 0;
    
    while(str[j] != '\0') {
        vidptr[i] = str[j];
        vidptr[i + 1] = 0x07;
        i = i + 2;
        j++;
    }
}

void print_line(char *str) {
    check_scroll();
    print_at(str, current_line * MAX_COLS);
    current_line++;
}

// ========== I/O FUNCTIONS ==========
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

// ========== STRING FUNCTIONS ==========
int strcmp(char *str1, char *str2) {
    int i = 0;
    while(str1[i] != '\0' && str2[i] != '\0') {
        if(str1[i] != str2[i]) {
            return 0;
        }
        i++;
    }
    return (str1[i] == '\0' && str2[i] == '\0');
}

int strlen(char *str) {
    int len = 0;
    while(str[len] != '\0') {
        len++;
    }
    return len;
}

void str_copy(char *dest, char *src) {
    int i = 0;
    while(src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int strncmp(char *str1, char *str2, int n) {
    for(int i = 0; i < n; i++) {
        if(str1[i] != str2[i]) {
            return 0;
        }
        if(str1[i] == '\0') {
            return 1;
        }
    }
    return 1;
}

// ========== SIMPLE FILESYSTEM ==========
#define MAX_FILES 20
#define MAX_FILENAME 32
#define MAX_FILESIZE 256

typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_FILESIZE];
    int size;
    int exists;
} File;

File filesystem[MAX_FILES];
int file_count = 0;

void init_filesystem() {
    for(int i = 0; i < MAX_FILES; i++) {
        filesystem[i].exists = 0;
        filesystem[i].size = 0;
    }
    file_count = 0;
    
    // Create some default files
    str_copy(filesystem[0].name, "readme.txt");
    str_copy(filesystem[0].content, "Welcome to Stefax OS! This is a simple file.");
    filesystem[0].size = strlen(filesystem[0].content);
    filesystem[0].exists = 1;
    file_count++;
    
    str_copy(filesystem[1].name, "info.txt");
    str_copy(filesystem[1].content, "Stefax OS - A simple operating system with filesystem support.");
    filesystem[1].size = strlen(filesystem[1].content);
    filesystem[1].exists = 1;
    file_count++;
}

void fs_list_files() {
    if(file_count == 0) {
        print_line("No files found.");
        return;
    }
    
    print_line("Files:");
    
    for(int i = 0; i < MAX_FILES; i++) {
        if(filesystem[i].exists) {
            check_scroll();
            print_at("  ", current_line * MAX_COLS);
            print_at(filesystem[i].name, current_line * MAX_COLS + 2);
            current_line++;
        }
    }
}

void fs_read_file(char *filename) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(filesystem[i].exists && strcmp(filesystem[i].name, filename)) {
            print_line("Content:");
            print_line(filesystem[i].content);
            return;
        }
    }
    
    print_line("File not found.");
}

void fs_create_file(char *filename, char *content) {
    // Check if file already exists
    for(int i = 0; i < MAX_FILES; i++) {
        if(filesystem[i].exists && strcmp(filesystem[i].name, filename)) {
            print_line("File already exists.");
            return;
        }
    }
    
    // Find empty slot
    for(int i = 0; i < MAX_FILES; i++) {
        if(!filesystem[i].exists) {
            str_copy(filesystem[i].name, filename);
            str_copy(filesystem[i].content, content);
            filesystem[i].size = strlen(content);
            filesystem[i].exists = 1;
            file_count++;
            
            print_line("File created successfully.");
            return;
        }
    }
    
    print_line("Filesystem full.");
}

void fs_delete_file(char *filename) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(filesystem[i].exists && strcmp(filesystem[i].name, filename)) {
            filesystem[i].exists = 0;
            filesystem[i].size = 0;
            file_count--;
            
            print_line("File deleted successfully.");
            return;
        }
    }
    
    print_line("File not found.");
}

// ========== KEYBOARD FUNCTIONS ==========
char scancode_to_ascii(unsigned char scancode) {
    char ascii_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        0, 0, ' '
    };
    
    if(scancode < 58) {
        return ascii_map[scancode];
    }
    return 0;
}

void read(char *buffer) {
    char *vidptr = (char*)0xb8000;
    unsigned int i = 0;
    unsigned int screen_pos = (current_line * 80 + 2) * 2;
    unsigned char scancode;
    char c;
    
    while(1) {
        while((inb(0x64) & 0x01) == 0);
        
        scancode = inb(0x60);
        
        if(scancode & 0x80) continue;
        
        c = scancode_to_ascii(scancode);
        
        if(c == '\n') {
            buffer[i] = '\0';
            current_line++;
            check_scroll();
            return;
        }
        
        if(c == '\b') {
            if(i > 0) {
                i--;
                screen_pos = screen_pos - 2;
                vidptr[screen_pos] = ' ';
                vidptr[screen_pos + 1] = 0x07;
            }
            continue;
        }
        
        if(c != 0) {
            buffer[i] = c;
            vidptr[screen_pos] = c;
            vidptr[screen_pos + 1] = 0x07;
            screen_pos = screen_pos + 2;
            i++;
        }
    }
}

// ========== COMMAND PARSING ==========
void parse_command(char *cmd, char *parts[], int *count) {
    *count = 0;
    int i = 0;
    int start = 0;
    
    while(cmd[i] != '\0') {
        if(cmd[i] == ' ') {
            cmd[i] = '\0';
            parts[*count] = &cmd[start];
            (*count)++;
            start = i + 1;
        }
        i++;
    }
    
    if(start < i) {
        parts[*count] = &cmd[start];
        (*count)++;
    }
}

// ========== SHELL FUNCTIONS ==========
void process_command(char *cmd) {
    char *parts[10];
    int part_count = 0;
    
    parse_command(cmd, parts, &part_count);
    
    if(part_count == 0) return;
    
    if(strcmp(parts[0], "help")) {
        print_line("Available commands:");
        print_line("  help           - Show this help");
        print_line("  clear          - Clear screen");
        print_line("  about          - About this OS");
        print_line("  ls             - List files");
        print_line("  cat <file>     - Read file");
        print_line("  write <file>  - Create file");
        print_line("  delete <file>  - Delete file");
	
    }
    else if(strcmp(parts[0], "clear")) {
        print("");
        current_line = 0;
        print_line("******************");
        print_line("    Stefax OS");
        print_line("******************");
    }
    else if(strcmp(parts[0], "about")) {
        print_line("Stefax OS v1.0");
        print_line("A basic kernel with filesystem");
    }
    else if(strcmp(parts[0], "ls")) {
        fs_list_files();
    }
    else if(strcmp(parts[0], "cat")) {
        if(part_count < 2) {
            print_line("Usage: cat <filename>");
        } else {
            fs_read_file(parts[1]);
        }
    }
    else if(strcmp(parts[0], "write")) {
        if(part_count < 2) {
            print_line("Usage: create <filename>");
        } else {
            print_line("Enter content: ");
            check_scroll();
            print_at("> ", current_line * MAX_COLS);
            
            char content[MAX_FILESIZE];
            read(content);
            
            fs_create_file(parts[1], content);
        }
    }
    else if(strcmp(parts[0], "delete")) {
        if(part_count < 2) {
            print_line("Usage: delete <filename>");
        } else {
            fs_delete_file(parts[1]);
        }
    }
    else if(cmd[0] != '\0') {
        print_line("Unknown command. Type 'help' for commands.");
    }
}

void shell() {
    char input[100];
    current_line=0;
    print_line("##########################");
    print_line("       STEFAX OS");
    print_line("##########################");
    print("");
    
    print_line("Initializing filesystem...");
    init_filesystem();
    print_line("Filesystem initialized!");
    print_line("");
    
    print_line("Welcome to Stefax OS!");
    print_line("Type 'help' for available commands");
    
    while(1) {
        check_scroll();
        print_at("> ", current_line * MAX_COLS);
        read(input);
        process_command(input);
    }
}

void kmain(void) {
    shell();
    return;
}
