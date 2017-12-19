//
// Created by Eli Eli on 18/11/2017.
//

#ifndef RIC_INTERFACE_BOARD_MANAGER_H
#define RIC_INTERFACE_BOARD_MANAGER_H

#include "communicator.h"
#include "timer.h"
#include "protocol.h"
#include "sensors_state.h"
#include <math.h>
#include <string.h>
#include <iostream>

namespace ric_interface
{
    class RicInterface
    {
    private:
        const int SEND_KA_TIMEOUT = 300; //ms
        const int GET_KA_TIMEOUT = 2000; //ms

        /* is board sent keep alive on time */
        bool is_board_alive_ = true, got_keepalive_ = false;
        Timer send_keepalive_timer_, get_keepalive_timer_;
        Communicator comm_;
        sensors_state sensors_state_;
        byte pkg_buff_[protocol::MAX_PKG_SIZE];

        void readAndHandlePkg();
        void checkKeepAliveFromRic();
        void sendKeepAlive();
        void clearBuffer();

    public:
        RicInterface();
        void connect(std::string port);
        void loop();
        bool isBoardAlive() { return is_board_alive_; }
        sensors_state getSensorsState() { return sensors_state_; }
        void writeCmd(const protocol::actuator &actu_pkg, size_t size, protocol::Type type);
    };
}



#endif //RIC_INTERFACE_BOARD_MANAGER_H
