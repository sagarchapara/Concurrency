namespace ConcurrentCollections
{
    internal class DelayQueue
    {

        private class ScheduledTask
        {
            public Action Action { get; private set; }

            public DateTimeOffset StartTime { get; private set; }

            public bool IsRecurring { get; private set; }

            public TimeSpan? Interval { get; private set; }

            public TimeSpan GetTimeDiff()
            {
                return StartTime.Subtract(DateTimeOffset.UtcNow);
            }

            public ScheduledTask(Action action, TimeSpan delay, bool isRecurring = false, TimeSpan? interval = null)
            {
                Action = action;
                StartTime = DateTimeOffset.UtcNow.Add(delay);
                IsRecurring = isRecurring;

                if (IsRecurring)
                {
                    if (interval == null)
                    {
                        throw new ArgumentNullException(nameof(interval));
                    }

                    Interval = interval;
                }
            }
        }

        PriorityQueue<ScheduledTask, double> queue;

        private readonly object _lockObj = new object();

        private volatile bool _isRunning = false;

        private Thread _parentThread;


        public DelayQueue()
        {
            this.queue = new PriorityQueue<ScheduledTask, double>();

            this.Start();
        }


        public void Schedule(Action action, TimeSpan delay, bool isRecurring = false, TimeSpan? interval = null)
        {
            lock (_lockObj)
            {
                ScheduledTask t = new ScheduledTask(action, delay, isRecurring, interval);

                queue.Enqueue(t, t.StartTime.Subtract(DateTimeOffset.UtcNow).TotalMilliseconds);

                if (queue.Peek() == t)
                {
                    _parentThread = null;

                    Monitor.Pulse(_lockObj);
                }
            }
        }

        public Action Poll()
        {
            lock (_lockObj)
            {
                while (_isRunning)
                {
                    if (queue.Count == 0)
                    {
                        Monitor.Wait(_lockObj);
                    }
                    else if (queue.Peek().GetTimeDiff().TotalMilliseconds > 0)
                    {
                        if (_parentThread != null)
                        {
                            Monitor.Wait(_lockObj);
                        }
                        else
                        {
                            _parentThread = Thread.CurrentThread;

                            try
                            {
                                Monitor.Wait(_lockObj, queue.Peek().GetTimeDiff());
                            }
                            finally
                            {
                                if (_parentThread == Thread.CurrentThread)
                                {
                                    _parentThread = null;
                                }
                            }
                        }
                    }
                    else
                    {
                        var t = queue.Dequeue();
                        return t.Action;
                    }

                    if (_parentThread == null && queue.Count > 0)
                    {
                        Monitor.Pulse(_lockObj);
                    }
                }
            }

            return null;
        }

        private void Executor()
        {
            while (_isRunning)
            {
                ScheduledTask t = null;

                lock (_lockObj)
                {
                    if (queue.Count == 0)
                    {
                        Monitor.Wait(_lockObj);
                    }
                    else if (queue.Peek().StartTime > DateTimeOffset.UtcNow)
                    {
                        if (_parentThread == null)
                        {
                            //make this _parentThread
                            _parentThread = Thread.CurrentThread;

                            try
                            {
                                Monitor.Wait(_lockObj, queue.Peek().GetTimeDiff());
                            }
                            finally
                            {
                                if (_parentThread == Thread.CurrentThread)
                                {
                                    //make parent thread null
                                    _parentThread = null;
                                }
                            }
                        }
                        else
                        {
                            //has _parentThread
                            Monitor.Wait(_lockObj);
                        }
                    }

                    if (queue.Count > 0 && queue.Peek().GetTimeDiff().TotalMilliseconds <= 0)
                    {
                        t = queue.Dequeue();

                        if (t.IsRecurring)
                        {
                            Schedule(t.Action, t.Interval.Value, true, t.Interval);
                        }
                    }

                    if (_parentThread == null && queue.Count > 0)
                    {
                        Monitor.Pulse(_lockObj);
                    }
                }

                if (t != null && _isRunning)
                {
                    //Execute the task
                    t.Action();
                }
            }
        }

        private void Start(int numWokers = 4)
        {
            lock (_lockObj)
            {
                if (_isRunning)
                {
                    return;
                }

                _isRunning = true;

                for (int i = 0; i < numWokers; i++)
                {
                    Thread t = new Thread(Executor);
                    t.Start();
                }
            }
        }

        private void Stop()
        {
            lock (_lockObj)
            {
                if (!_isRunning)
                {
                    return;
                }

                _isRunning = false;

                Monitor.PulseAll(_lockObj);
            }
        }
    }
}
