//
// Created by Russell Johnson on 3/24/25.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <list>


#define MAX_NUMBER 100000
#define THREAD_COUNT 10

std::mutex numberAccess;
std::mutex primeAccess;

void isPrimeT(std::vector<int> &primeV, std::list<int> &numL) {
    bool numsLeft = true;
    while (numsLeft) {
        // find next number in the number list
        numberAccess.lock();
        if(numL.empty()) { // skip all this if there are no number left
            numberAccess.unlock();
            numsLeft = false;
        }
        else {
            int number = numL.front();
            numL.pop_front();
            numberAccess.unlock();


            bool isPrime = true;
            for (int i = 2; i < number && isPrime; i++) {
                if (number % i == 0) isPrime = false;;
            }
            if (isPrime) {
                primeAccess.lock();
                primeV.push_back(number);
                primeAccess.unlock();
            }
        }
    }
}

void isPrimeS(std::vector<int> &primeV, std::list<int> &numL) {
    while (numL.size() > 0) {
        int number = numL.front();
        numL.pop_front();

        bool isPrime = true;
        for (int i = 2; i < number && isPrime; i++) {
            if (number % i == 0) isPrime = false;;
        }
        if(isPrime) primeV.push_back(number);
    }
}

int main() {
    std::vector<int> primeV1;
    std::vector<int> primeV2;

    std::list<int> numberL1;
    std::list<int> numberL2;
    for (int i = 3; i < MAX_NUMBER; i++) {
        numberL1.push_back(i);
        numberL2.push_back(i);
    }

    std::thread primeFinders[THREAD_COUNT];
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < THREAD_COUNT; i++) {
        primeFinders[i] = std::thread(isPrimeT, std::ref(primeV1), std::ref(numberL1));
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        primeFinders[i].join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << std::endl;
    for(int i : primeV1){
        std::cout << i << " ";
    }
    std::cout << std::endl << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    isPrimeS(primeV2, numberL2);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << std::endl;
    for(int i : primeV2){
        std::cout << i << " ";
    }
    std::cout << std::endl << "Time taken: " << duration.count() << " milliseconds" << std::endl;

}