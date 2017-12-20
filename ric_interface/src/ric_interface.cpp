
#include <ric_interface/ric_interface.h>


namespace ric_interface
{
    RicInterface::RicInterface()
    {

    }

    /* open connection to serial port                */
    /* if conncetion fails, exception will be thrown */
    void RicInterface::connect(std::string port)
    {
        comm_.connect(port, 500000);
    }

    void RicInterface::clearBuffer()
    {
        memset(pkg_buff_, 0, protocol::MAX_PKG_SIZE);
    }

    void RicInterface::loop()
    {
        sendKeepAlive();
        readAndHandlePkg();
        checkKeepAliveFromRic();
    }

    void RicInterface::checkKeepAliveFromRic()
    {
        get_keepalive_timer_.startTimer(GET_KA_TIMEOUT);
        if (get_keepalive_timer_.isFinished())
        {
            if (got_keepalive_) //connected
            {
                is_board_alive_ = true;
                got_keepalive_ = false;
                //fprintf(stderr,"board alive ! \n");
            }
            else
            {
                is_board_alive_ = false;
                fprintf(stderr,"board dead ! \n");
            }
            get_keepalive_timer_.reset();
        }
    }

    void RicInterface::readAndHandlePkg()
    {
        int pkg_type = comm_.read(pkg_buff_);
        if (pkg_type != -1)
        {
            switch (pkg_type)
            {
                case (uint8_t) protocol::Type::KEEP_ALIVE:
                {
                    protocol::keepalive ka_pkg;
                    Communicator::fromBytes(pkg_buff_, sizeof(protocol::keepalive), ka_pkg);
                    if (ka_pkg.type == (uint8_t)protocol::Type::KEEP_ALIVE)
                    {
                        //fprintf(stderr, "got keep alive\n");
                        got_keepalive_ = true;
                    }
                    break;
                }
                case (uint8_t) protocol::Type::LOGGER:
                {
                    protocol::logger logger_pkg;
                    Communicator::fromBytes(pkg_buff_, sizeof(protocol::logger), logger_pkg);
                    if (logger_pkg.type == (uint8_t)protocol::Type::LOGGER)
                    {
                        sensors_state_.logger = logger_pkg;
                        //fprintf(stderr, "logger msg: %s, code: %d\n", logger_pkg.msg, logger_pkg.value);
                    }

                    break;
                }
                case (int)protocol::Type::ULTRASONIC:
                {
                    protocol::ultrasonic ultrasonic_pkg;
                    Communicator::fromBytes(pkg_buff_, sizeof(protocol::ultrasonic), ultrasonic_pkg);
                    if (ultrasonic_pkg.type == (uint8_t)protocol::Type::ULTRASONIC)
                    {
                        sensors_state_.ultrasonic = ultrasonic_pkg;
                        fprintf(stderr, "ultrasonic: %d\n", ultrasonic_pkg.distance_mm);
                    }
                    break;
                }
                case (int)protocol::Type::IMU:
                {
                    protocol::imu imu_pkg;
                    Communicator::fromBytes(pkg_buff_, sizeof(protocol::imu), imu_pkg);
                    if (imu_pkg.type == (uint8_t)protocol::Type::IMU)
                    {
                        sensors_state_.imu = imu_pkg;
                        /*fprintf(stderr, "imu: roll: %f, pitch: %f, yaw: %f \n", sensors_state_.imu.roll_rad * 180 / M_PI,
                                sensors_state_.imu.pitch_rad * 180 / M_PI,
                                sensors_state_.imu.yaw_rad * 180 / M_PI);*/
                    }
                    break;
                }
                case (int)protocol::Type::LASER:
                {
                    protocol::laser laser_pkg;
                    Communicator::fromBytes(pkg_buff_, sizeof(protocol::laser), laser_pkg);
                    if (laser_pkg.type == (uint8_t)protocol::Type::LASER)
                    {
                        sensors_state_.laser = laser_pkg;
                        //fprintf(stderr, "laser dist: %d\n", sensors_state_.laser.distance_mm);
                    }
                    break;
                }
                case (int)protocol::Type::GPS:
                {
                    protocol::gps gps_pkg;
                    Communicator::fromBytes(pkg_buff_, sizeof(protocol::gps), gps_pkg);
                    if (gps_pkg.type == (uint8_t)protocol::Type::GPS)
                    {
                        sensors_state_.gps = gps_pkg;
                        //fprintf(stderr,"gps lat: %f, lon: %f\n", sensors_state_.gps.lat, sensors_state_.gps.lon);
                    }
                    break;
                }
            }
            clearBuffer();
        }
    }

    void RicInterface::sendKeepAlive()
    {
        send_keepalive_timer_.startTimer(SEND_KA_TIMEOUT);
        if (send_keepalive_timer_.isFinished())
        {
            //puts("sending ka");
            protocol::keepalive ka_pkg;
            comm_.write(ka_pkg, sizeof(protocol::keepalive));
            send_keepalive_timer_.reset();
        }
    }

    void RicInterface::writeCmd(const protocol::actuator &actu_pkg, size_t size, protocol::Type type)
    {
        comm_.write(actu_pkg, size);
    }
}