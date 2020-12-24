/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package Beatmup.Visual;

import java.util.HashMap;
import java.util.Map;


/**
 * Managing timed events
 */
public class Animator {
    public static abstract class Event {
        /**
         * Performs an event
         * @return the time in milliseconds in which the event must be invoked again; a negative value means "never"
         */
        public abstract long run();
    }

    private static final Object lock = new Object();    //!< main access control
    private static Animator instance = null;            //!< the instance

    private boolean threadRunning;              //!< while `true`, the thread is running
    private final Thread thread;                //!< the animator thread
    private Map<Integer, Event> events;         //!< list of planned events, indexed by ticket numbers
    private Map<Integer, Long> eventTimes;      //!< list of time points when the planned events are, indexed by ticket numbers
    private int ticketCounter;                  //!< counter issuing new ticket numbers
    private long nextWakeUpTime;                //!< time point when the thread is waking up

    public static final int INVALID_TICKET = -123456789;
    private static final long TIME_TOLERANCE = 3;

    private Animator() {
        events = new HashMap<>();
        eventTimes = new HashMap<>();
        threadRunning = true;
        ticketCounter = 0;
        nextWakeUpTime = 0;
        thread = new Thread(new Runnable() {
            @Override
            public void run() {
                while (threadRunning) {
                    try {
                        threadBody();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }, "Beatmup display animator" );
        thread.start();
    }


    private void threadBody() throws InterruptedException {
        // if there is no events, just sleep
        if (events.isEmpty())
            synchronized (thread) {
                thread.wait();
            }

        // pick an event first
        Event toDoNow = null;
        int id = INVALID_TICKET;
        final long now = getCurrentTime();
        long earliestEvent = Long.MAX_VALUE;
        synchronized (lock) {
            for (Map.Entry<Integer, Long> entry : eventTimes.entrySet()) {
                long time = entry.getValue();
                earliestEvent = Math.min(earliestEvent, time);
                if (time <= now + TIME_TOLERANCE) {
                    id = entry.getKey();
                    toDoNow = events.get(id);
                    break;
                }
            }
        }

        // no event to do right now, sleep and quit
        if (toDoNow == null) {
            nextWakeUpTime = earliestEvent;
            long delay = nextWakeUpTime - getCurrentTime();
            if (delay > 0)
                synchronized (thread) {
                    thread.wait(delay);
                }
            return;
        }

        // there is an event, run it
        long next = toDoNow.run();
        while (0 <= next && next <= TIME_TOLERANCE)
            next = toDoNow.run();
        synchronized (lock) {
            if (next > 0)
                eventTimes.put(id, getCurrentTime()+next);
            else {
                events.remove(id);
                eventTimes.remove(id);
            }
        }
    }


    private long getCurrentTime() {
        return System.currentTimeMillis();
    }


    /**
     * Initializes the animator, if needed, and returns its instance
     * @return the Animator instance
     */
    public static Animator getInstance() {
        synchronized (lock) {
            if (instance == null)
                instance = new Animator();
            return instance;
        }
    }


    /**
     * Pushes an event to the queue, assigning a unique ticket
     * @param event the event
     * @param time time in milliseconds to invoke the event
     * @return the ticket number
     */
    public int planify(Event event, int time) {
        final long now = getCurrentTime();
        if (time >= 0)
            synchronized (lock) {
                // adding new event
                long plannedTime = now + time;
                events.put(ticketCounter, event);
                eventTimes.put(ticketCounter, plannedTime);
                // if the thread sleeps at the planned time point, wake it up now
                if (plannedTime < nextWakeUpTime || nextWakeUpTime < now)
                    synchronized (thread) {
                        thread.notify();
                    }
                return ticketCounter++;
            }
        return INVALID_TICKET;
    }

    /**
     * Cancels a planified event
     * @param event the event to cancel
     * @param ticket its ticket number
     * @return `true` if the event is cancelled successfully
     */
    public boolean cancel(Event event, int ticket) {
        if (ticket != INVALID_TICKET && event != null)
            synchronized (lock) {
                if (events.remove(ticket) == event) {
                    eventTimes.remove(ticket);
                    return true;
                }
            }
        return false;
    }
}