//
// Created by Russell Johnson on 3/24/25.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <list>
#include <iomanip>

#define MAX_NUMBER 100000
#define THREAD_COUNT 4

std::mutex numberAccess;
std::mutex primeAccess;


/** This is a very inefficient method to determine if a number is prime.
 *
 * @param primeV reference to the output vector of prime numbers
 * @param numL reference to the input list of numbers to check
 */
void isPrimeT(std::vector<int> &primeV, std::list<int> &numL) {
    bool numsLeft = true;
    while (numsLeft) {
        // find next number in the number list
        numberAccess.lock();
        if (numL.empty()) { // skip all this if there are no number left
            numberAccess.unlock();
            numsLeft = false;
        } else {
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

/** Same as above, but done sequentially instead of parallel
 *
 * @param primeV reference to the output vector of prime numbers
 * @param numL reference to the input list of numbers to check
 */
void isPrimeS(std::vector<int> &primeV, std::list<int> &numL) {
    while (numL.size() > 0) {
        int number = numL.front();
        numL.pop_front();

        bool isPrime = true;
        for (int i = 2; i < number && isPrime; i++) {
            if (number % i == 0) isPrime = false;;
        }
        if (isPrime) primeV.push_back(number);
    }
}

/** displays a progress bar
 *
 * @param p progress made
 * @param t total task size
 */
void dispProg(int p, int t) {
    int width = 50;
    float percent = (float) p / t;
    int pos = width * percent;

    std::cout << "\r[";
    for (int i = 0; i < width; ++i) {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << "|";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(percent * 100.0) << " %" << std::flush;
}

/** thread that checks progress every .1 seconds
 *
 * @param numL list of numbers being worked on
 */
void progT(std::list<int> &numL) {
    while (numL.size() > 0) {
        dispProg(MAX_NUMBER - numL.size(), MAX_NUMBER);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::vector<int> primeV1;
    std::vector<int> primeV2;

    primeV1.reserve(MAX_NUMBER);

    std::list<int> numberL1;
    std::list<int> numberL2;
    for (int i = 3; i < MAX_NUMBER; i++) {
        numberL1.push_back(i);
        numberL2.push_back(i);
    }


    std::cout << std::endl << "Finding primes between 2-" << MAX_NUMBER
              << " with sequential checking: " << std::endl;
    std::thread sProgTracker(progT, std::ref(numberL2));
    auto start = std::chrono::high_resolution_clock::now();
    isPrimeS(primeV2, numberL2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    sProgTracker.join();
    std::cout << std::endl << "First 100 primes found: ";
    for (int i = 0; i < primeV2.size() && i < 100; i++) {
        std::cout << primeV2[i] << " ";
    }
    std::cout << std::endl << "Time taken: " << duration2.count() << " milliseconds" << std::endl;

    std::thread primeFinders[THREAD_COUNT];
    std::cout << std::endl << "Finding primes between 2-" << MAX_NUMBER
              << " with " << THREAD_COUNT << " threads checking in parallel: " << std::endl;
    std::thread tProgTracker(progT, std::ref(numberL1));
    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < THREAD_COUNT; i++) {
        primeFinders[i] = std::thread(isPrimeT, std::ref(primeV1), std::ref(numberL1));
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        primeFinders[i].join();
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    tProgTracker.join();
    std::cout << std::endl << "First 100 primes found: ";
    for (int i = 0; i < primeV1.size() && i < 100; i++) {
        std::cout << primeV1[i] << " ";
    }
    std::cout << std::endl << "Time taken: " << duration1.count() << " milliseconds" << std::endl;

    int timeDiff = duration2.count() - duration1.count();
    float fasterFactor = (float) duration2.count() / (float) duration1.count();
    std::cout << std::endl << "Time difference is " << timeDiff << " milliseconds or ";
    std::cout << std::setprecision(2);
    std::cout << fasterFactor << "x faster." << std::endl;

}
