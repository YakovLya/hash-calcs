#include <iostream>
#include <cstdint>
#include <cstring>
#include <bitset>
#include <iomanip>

using namespace std;

uint32_t h[8] = { // first 32 bits of square roots of first 8 primes
    0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
    0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
};

const uint32_t k[64] = { // first 32 bits of cube roots of first 64 primes
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

uint32_t rotr(uint32_t a, uint32_t k){ // right rotate, like shift (>>) but with cycle for 32bits length
    return (a >> k) | (a << (32 - k));
}

string dec_to_bin(uint32_t num){
    string bin = "";
    while(num > 0){
        bin = (num % 2 ? "1" : "0") + bin;
        num /= 2;
    }
    return bin;
}

string dec_to_hex(uint32_t num){
    stringstream stream;
    stream << setfill('0') << setw(8) << hex << num;
    return stream.str();
}

string prepare_msg(string msg){
    int length = msg.size();
    string prepared = "";
    for(char &c: msg) // convert string to string of bits 
        prepared += bitset<8>(c).to_string();
    prepared += '1';
    string length_bin = dec_to_bin(length * 8);
    int padding = (448 + 512 * (prepared.size() / 448)) - prepared.size() + (64 - length_bin.size()); // how many '0' need to write in the end
    while(padding --)
        prepared += '0';
    prepared += length_bin; // write length of the original message to the end (bei)
    return prepared;
}

string calc_hash(string msg){
    string completed_msg = prepare_msg(msg);
    int blocks_count = completed_msg.size() / 512; // block size = 512
    for(int block_num = 0; block_num < blocks_count; block_num ++){
        string block = completed_msg.substr(block_num * 512, 512); 

        uint32_t word[64]; // [0..15] are from block, others are calculated
        for(int word_num = 0; word_num < 16; word_num ++) // split each block to 16 blocks 32 bits size
            word[word_num] = stoi(block.substr(word_num * 32, 32), 0, 2);
        for(int word_num = 16; word_num < 64; word_num ++){ // calculate [16..63] words by the formula
            uint32_t x0 = rotr(word[word_num - 15], 7) ^ rotr(word[word_num - 15], 18) ^ (word[word_num - 15] >> 3);
            uint32_t x1 = rotr(word[word_num -2], 17) ^ rotr(word[word_num - 2], 19) ^ (word[word_num - 2] >> 10);
            word[word_num] = word[word_num - 16] + x0 + word[word_num - 7] + x1;
        }

        uint32_t a[8]; // copy of h[0..7] for calculations
        copy(begin(h), end(h), begin(a));

        for(int i = 0; i < 64; i ++){ // calculations 
            uint32_t sum0 = rotr(a[0], 2) ^ rotr(a[0], 13) ^ rotr(a[0], 22);
            uint32_t sum1 = rotr(a[4], 6) ^ rotr(a[4], 11) ^ rotr(a[4], 25);
            uint32_t maj = (a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]);
            uint32_t choi = (a[4] & a[5]) ^ ((~a[4]) & a[6]);
            uint32_t temp0 = a[7] + sum1 + choi + k[i] + word[i];
            uint32_t temp1 = sum0 + maj;

            for(int j = 7; j >= 0; j --){
                if (j == 4)
                    a[j] = a[j - 1] + temp0;
                else if (j == 0)
                    a[j] = temp0 + temp1;
                else
                    a[j] = a[j - 1];
            }
        }

        for(int i = 0; i < 8; i ++) // add calculations to hash values
            h[i] += a[i];
    }

    string hash = "";
    for(int i = 0; i < 8; i ++) // merge to hash
        hash += dec_to_hex(h[i]);
    
    return hash;
}




main(){
    cout << calc_hash("hello world"); // b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9
}