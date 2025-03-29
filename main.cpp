//
// Created by Russell Johnson on 3/24/25.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <list>
#include <iomanip>
#include <mutex>

std::mutex numberAccess;
std::mutex primeAccess;

int MAX_NUMBER = 100000;
int THREAD_MAX = 4;
int THREAD_COUNT = 4;

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
    dispProg(1, 1); // added this so it always ends at 100%

}

void resetList(std::list<int> &numL){
    numL.clear();
    for (int i = 3; i < MAX_NUMBER; i++) {
        numL.push_back(i);
    }
}

int main() {

    std::cout << "Checking a range of numbers to see if they are prime." << std::endl;
    std::cout << "Enter the upper bound of range without commas:" << std::endl;
    std::cout << "(above 200,000 may be very slow, below 50,000 may not take long enough to give meaningful results): ";
    std::cin >> MAX_NUMBER;
    std::cout << "Enter the max number of threads to be used in a test: ";
    std::cin >> THREAD_MAX;

    std::vector<int> primeV;
    primeV.reserve(MAX_NUMBER);
    std::list<int> numberL;
    std::vector<std::chrono::milliseconds> durations;
    resetList(numberL);

    // single thread
    std::cout << std::endl << "Establishing Baseline with non-multithreading test" << std::endl;
    std::cout << "Finding primes between 2-" << MAX_NUMBER
              << " with sequential checking: " << std::endl;
    std::thread sProgTracker(progT, std::ref(numberL));
    auto start = std::chrono::high_resolution_clock::now();
    isPrimeS(primeV, numberL);
    auto end = std::chrono::high_resolution_clock::now();
    durations.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
    //auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    sProgTracker.join();
    std::cout << std::endl << "First 10 primes found: ";
    for (int i = 0; i < primeV.size() && i < 10; i++) {
        std::cout << primeV[i] << " ";
    }
    std::cout << " Largest prime found: " << primeV.back();
    std::cout << std::endl << "Time taken: " << durations.back().count() << " milliseconds" << std::endl;

    // multi thread tests
    for( THREAD_COUNT = 2; THREAD_COUNT <= THREAD_MAX; THREAD_COUNT++) {
        resetList(numberL);
        primeV.clear();
        std::thread primeFinders[THREAD_COUNT];
        std::cout << std::endl << "Finding primes between 2-" << MAX_NUMBER
                  << " with " << THREAD_COUNT << " threads checking in parallel: " << std::endl;
        std::thread tProgTracker(progT, std::ref(numberL));
        start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < THREAD_COUNT; i++) {
            primeFinders[i] = std::thread(isPrimeT, std::ref(primeV), std::ref(numberL));
        }
        for (int i = 0; i < THREAD_COUNT; i++) {
            primeFinders[i].join();
        }
        end = std::chrono::high_resolution_clock::now();
        durations.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
        tProgTracker.join();
        std::cout << std::endl << "First 10 primes found: ";
        for (int i = 0; i < primeV.size() && i < 10; i++) {
            std::cout << primeV[i] << " ";
        }
        std::cout << " Largest prime found: " << primeV.back();
        std::cout << std::endl << "Time taken: " << durations.back().count() << " milliseconds" << std::endl;

        int timeDiff = durations.front().count() - durations.back().count();
        float fasterFactor = (float) durations.front().count() / (float) durations.back().count();
        if(timeDiff > 0) {
            std::cout << "Time difference is " << timeDiff << " milliseconds or ";
            std::cout << std::setprecision(2);
            std::cout << fasterFactor << "x faster." << std::endl;
        }
        else std::cout << "No significant speed increase." << std::endl;
    }
}
