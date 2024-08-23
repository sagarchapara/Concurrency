namespace ConcurrentCollections
{
    internal class ConcurrentStack<T>
    {
        private class Node
        {
            public T Value { get; set; }
            public Node Next { get; set; }

            public Node(T value)
            {
                this.Value = value;
            }
        }

        private volatile Node Head;

        public ConcurrentStack() { }

        public void Push(T value)
        {
            Node next = new Node(value);

            next.Next = Head;

            if (Interlocked.CompareExchange(ref Head, next, next.Next) != next.Next)
            {
                return;
            }

            //If we are not able to push it in first time, let's use a spin wait
            PushInternal(next);
        }

        public bool TryPeek(out T value)
        {
            Node m_val = Head;

            if (m_val == null)
            {
                value = default(T);
                return false;
            }

            value = m_val.Value;
            return true;
        }

        public bool TryPop(out T value)
        {
            Node m_val = Head;

            if (m_val == null)
            {
                value = default(T);
                return false;
            }

            if (Interlocked.CompareExchange(ref Head, m_val.Next, m_val) != m_val)
            {
                value = m_val.Value;
                return true;
            }

            //If we are unable to pop it first time, let's use spin wait.
            return PopInternal(out value);
        }

        private void PushInternal(Node node)
        {
            SpinWait spinWait = new SpinWait();

            do
            {
                spinWait.SpinOnce();

                node.Next = Head;
            } while (Interlocked.CompareExchange(ref Head, node, node.Next) != node.Next);

        }

        private bool PopInternal(out T value)
        {
            SpinWait spinWait = new SpinWait();

            Node curr;

            do
            {
                spinWait.SpinOnce();

                curr = Head;

                if (curr == null)
                {
                    value = default(T);
                    return false;
                }


            } while (Interlocked.CompareExchange(ref Head, curr.Next, curr) != curr);

            value = curr.Value;
            return true;
        }
    }
}
