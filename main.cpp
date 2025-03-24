//
// Created by Russell Johnson on 3/24/25.
//

#include <iostream>
#include <thread>

std::mutex arrayAccess;

void isPrime(int *primePoint, int number) {
    bool isPrime = true;
    for (int i = 2; i < number; i++) {
        if (number % i == 0) return;;
    }
    // TODO put a mutex and a global in here to handle writing to the right index
    int index = 0;
    while (index < 100) {
        if (*primePoint == 0) {
            arrayAccess.lock();
            *primePoint = number;
            std::cout << number << std::endl;
            arrayAccess.unlock();
            return;
        }
        primePoint++;
        index++;
    }
}

int main() {
    int threadCount = 100;

    int primeArray[100];
    std::thread primeFinders[threadCount];
    int number = 3;
    for (int i = 0; i < threadCount; i++) {
        primeFinders[i] = std::thread(isPrime, &primeArray[0], number);
        number++;
    }
    for (int i = 0; i < threadCount; i++) {
        primeFinders[i].join();
    }

}