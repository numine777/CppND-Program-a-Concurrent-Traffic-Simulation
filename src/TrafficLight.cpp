#include <iostream>
#include <random>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mtx);
    _cond.wait(uLock, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> gLock(_mtx);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TrafficLightPhase currPhase = _phases.receive();
        if (currPhase == green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    int duration = std::rand() % (6-4 + 1) + 4;

    auto lastUpdate = std::chrono::system_clock::now();

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        long timePassed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()- lastUpdate).count();

        if (timePassed >= duration) {

            if (_currentPhase == green) {
                _currentPhase = red;
            }
            else if (_currentPhase == red) {
                _currentPhase = green;
            }

            _phases.send(std::move(_currentPhase));
            lastUpdate = std::chrono::system_clock::now();
            duration = std::rand() % (6-4 + 1) + 4;
        }
    }
}

