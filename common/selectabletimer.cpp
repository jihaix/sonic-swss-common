#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <system_error>

#include "common/logger.h"
#include "common/selectabletimer.h"

namespace swss {

SelectableTimer::SelectableTimer(const timespec& interval, int pri)
    : Selectable(pri), m_zero({{0, 0}, {0, 0}})
{
    // Create the timer
    m_tfd = timerfd_create(CLOCK_REALTIME, 0);
    if (m_tfd == -1)
    {
        SWSS_LOG_THROW("failed to create timerfd, errno: %s", strerror(errno));
    }
    setInterval(interval);
}

SelectableTimer::~SelectableTimer()
{
    int err;

    do
    {
        err = close(m_tfd);
    }
    while(err == -1 && errno == EINTR);
}

void SelectableTimer::start()
{
    // Set the timer interval and the timer is automatically started
    int rc = timerfd_settime(m_tfd, 0, &m_interval, NULL);
    if (rc == -1)
    {
        SWSS_LOG_THROW("failed to set timerfd, errno: %s", strerror(errno));
    }
}

void SelectableTimer::stop()
{
    // Set the timer interval and the timer is automatically started
    int rc = timerfd_settime(m_tfd, 0, &m_zero, NULL);
    if (rc == -1)
    {
        SWSS_LOG_THROW("failed to set timerfd to zero, errno: %s", strerror(errno));
    }
}

void SelectableTimer::reset()
{
    stop();
    start();
}

void SelectableTimer::setInterval(const timespec& interval)
{
    // The initial expiration and intervals to caller specified
    m_interval.it_value = interval;
    m_interval.it_interval = interval;
}

int SelectableTimer::getFd()
{
    return m_tfd;
}

void SelectableTimer::readData()
{
    uint64_t r = UINT64_MAX;

    ssize_t s;
    errno = 0;
    do
    {
        s = read(m_tfd, &r, sizeof(uint64_t));
    }
    while(s == -1 && errno == EINTR);

    ABORT_IF_NOT(s == sizeof(uint64_t), "Failed to read timerfd. s=%ld", s)

    // r = count of timer events happened since last read.
}

}
