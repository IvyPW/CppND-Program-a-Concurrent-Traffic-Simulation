#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <thread>
#include <future>

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    std::unique_lock<std::mutex> uLock(_mutex);
    //pass a Lambda to wait(), which repeatedly checks wether the vector contains elements (thus the inverted logical expression):
    _condition.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable
    // remove the first element from queue?
    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> uLock(_mutex);

    // add msg to the back of queue?
    //std::cout << "   Message" << msg << " has been sent to the queue" << std::endl;

    //_queue.push_back(std::move(msg));
    //Use emplace_back directly here as inside its implementation it moves the obj
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == TrafficLightPhase::green)
            return;
    }
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    //TODO:
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    while(true){
        //Consider using the Mersenne twister algorithm based mt19937 library to generate a random number.
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(4,6);
        int cycle_duration = distribution(generator);  // generates number in the range 1..6
        std::this_thread::sleep_for(std::chrono::seconds(cycle_duration));
        switch (_currentPhase){
            case(TrafficLightPhase::red): _currentPhase = TrafficLightPhase::green;break;
            case(TrafficLightPhase::green):_currentPhase = TrafficLightPhase::red;break;
        }
        _messageQueue.send(std::move(_currentPhase));
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}