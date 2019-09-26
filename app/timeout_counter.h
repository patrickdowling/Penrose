/*
 *  Copyright 2019 Tyler Coy
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PENROSE_APP_TIMEOUT_COUNTER_H_
#define PENROSE_APP_TIMEOUT_COUNTER_H_

namespace penrose
{

template <typename T, T timeout>
class TimeoutCounter
{
  protected:
    T count_;

  public:
    void Init(void)
    {
        Start();
    }

    void Start(void)
    {
        count_ = timeout;
    }

    bool Tick(void)
    {
        bool timed_out = (count_ == 1);

        if (count_ > 0)
        {
            count_--;
        }

        return timed_out;
    }
};

}

#endif
