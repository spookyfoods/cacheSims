#include <bitset>
#include <iostream>
#include <utility>
#include <vector>
#include <iomanip>
#include <string>
#include <stdexcept>

#define OFFSET 5
#define INDEX 5
#define TAG 22

std::string describeStatusCode(int statusCode){
    if(statusCode==4){
        return "";
    }else{
        if(statusCode==2){
            return "Compulsory Miss";
        }else if(statusCode==1){
            return "Conflict Miss";
        }
    }
    return "Unexpected Status Code";
}

struct CacheLine {
    bool valid_bit;
    uint32_t tag;
    CacheLine() : valid_bit(false), tag(0) {}
};

class DirectMappedCache {
private:
    std::vector<CacheLine> blocks;

    uint32_t getOffset(uint32_t hum){
        return((hum)&((1 << OFFSET)-1));
    }
    uint32_t getIndex(uint32_t hum){
        hum >>= OFFSET;
        return((hum)&((1 << INDEX)-1));
    }
    uint32_t getTag(uint32_t hum){
        hum >>= (OFFSET + INDEX);
        return((hum)&((1 << TAG)-1));
    }

public:
    DirectMappedCache() : blocks(32) {};

    int read(uint32_t address) {
        uint32_t tag = getTag(address);
        uint32_t index = getIndex(address);

        CacheLine& ourLine = blocks[index];

        if(ourLine.valid_bit && tag == ourLine.tag) {
            // It's a Hit
            return 4;
        } else {
            // It's a Miss (either compulsory or conflict)
            int miss_type;
            if(!ourLine.valid_bit) {
                // Compulsory miss
                miss_type = 2;
            } else {
                // Conflict miss
                miss_type = 1;
            }

            ourLine.valid_bit = true;
            ourLine.tag = tag;
            

            return miss_type;
        }
    }
};

int main(int argc, char* argv[]){
    
    uint32_t addy;
    std::vector<uint32_t> addresses{};
    for(int i{1};i<argc;i++){
        try{
            addy=std::stoul(std::string(argv[i]), nullptr, 0);
            addresses.push_back(addy);
        } catch(const std::exception& e){
            std::cerr << "Error parsing addresses: " << e.what() << std::endl;
            std::cerr << "Please provide valid numbers (e.g., 0x140A0 or 82080)." << std::endl;
            return 1;
        }
    }

    DirectMappedCache myCache;

    auto testRead = [&](uint32_t addr) -> std::pair<bool, int> {
        int hit = myCache.read(addr);
        if(hit > 3) { // 4 is a hit
            return {true, hit};
        } else { // 1 or 2 is a miss
            return {false, hit};
        }
    };
    for(int i{0};i<addresses.size();i++){
        std::cout << "Test " << i << "; Reading " << std::hex
         << std::showbase << addresses[i] << std::dec << '\n';
        auto result = testRead(addresses[i]);
        std::cout << " Hit: " << std::boolalpha << result.first << ", Status Code "
        << result.second << '('<<describeStatusCode(result.second) << ')'<<"\n\n";
    }    
    return 0;
}
