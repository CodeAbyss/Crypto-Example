#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "base64.h"
#include "Crypto.h"

//#define CONVERT_TO_BASE64

using namespace std;

void writeFile(char *filename, unsigned char *file, size_t fileLength);
int readFile(char *filename, unsigned char **file);


int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "No file argument supplied.\n");
        return 1;
    }

    char *filename = argv[1];

    // Create our crypto object
    Crypto crypto;

    // Read the file to encrypt
    unsigned char *file;
    size_t fileLength = readFile(filename, &file);

    // Encrypt the file
    unsigned char *encryptedFile;
    int encryptedFileLength;
    if((encryptedFileLength = crypto.aesEncrypt((const unsigned char*)file, fileLength, &encryptedFile)) == -1) {
        fprintf(stderr, "Encryption failed\n");
        return 1;
    }
    printf("%d bytes encrypted\n", encryptedFileLength);

    // Append .enc to the filename
    char *encryptedFilename = (char*)malloc(strlen(filename) + 5);
    sprintf(encryptedFilename, "%s.enc", filename);

    #ifdef CONVERT_TO_BASE64
        // Encode to encrypted file to base64
        char *base64Buffer;
        base64Buffer = base64Encode(encryptedFile, encryptedFileLength);

        // Change the encrypted file pointer to the base64 string and update
        // the length (we can use strlen() now since the base64 string is ASCII data)
        free(encryptedFile);
        encryptedFile = (unsigned char*)base64Buffer;
        encryptedFileLength = strlen((char*)encryptedFile);
    #endif
    
    // Write the encrypted file to its own file
    writeFile(encryptedFilename, encryptedFile, encryptedFileLength);
    printf("Encrypted message written to \"%s\"\n", encryptedFilename);

    free(file);

    // Read the encrypted file back
    fileLength = readFile(encryptedFilename, &file);
    printf("read %d bytes\n", (int)fileLength);

    #ifdef CONVERT_TO_BASE64
        // Decode the encrypted file from base64
        unsigned char *binaryBuffer;
        fileLength = base64Decode((char*)file, &binaryBuffer);

        // Change the pointer of the string containing the file info to the decoded base64 string
        free(file);
        file = binaryBuffer;
    #endif

    // Decrypt the encrypted file
    unsigned char *decryptedFile;
    int decryptedFileLength;
    if((decryptedFileLength = crypto.aesDecrypt(file, fileLength, &decryptedFile)) == -1) {
        fprintf(stderr, "Decryption failed\n");
        return 1;
    }

    // Append .dec to the filename
    char *decryptedFilename = (char*)malloc(strlen(filename) + 5);
    sprintf(decryptedFilename, "%s.dec", filename);
    
    // Write the decrypted file to its own file
    writeFile(decryptedFilename, decryptedFile, decryptedFileLength);
    printf("Decrypted file written to \"%s\"\n", decryptedFilename);

    free(decryptedFile);
    free(file);

    return 0;
}


void writeFile(char *filename, unsigned char *file, size_t fileLength) {
    FILE *fd = fopen(filename, "wb");
    if(fd == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        exit(1);
    }

    size_t bytesWritten = fwrite(file, 1, fileLength, fd);

    if(bytesWritten != fileLength) {
        fprintf(stderr, "Failed to write file\n");
        exit(1);
    }

    fclose(fd);
}


int readFile(char *filename, unsigned char **file) {
    FILE *fd = fopen(filename, "rb");
    if(fd == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        exit(1);
    }

    // Determine size of the file
    fseek(fd, 0, SEEK_END);
    size_t fileLength = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    // Allocate space for the file
    *file = (unsigned char*)malloc(fileLength);

    // Read the file into the buffer
    size_t bytesRead = fread(*file, 1, fileLength, fd);

    if(bytesRead != fileLength) {
        fprintf(stderr, "Error reading file\n");
        exit(1);
    }

    fclose(fd);

    return fileLength;
}