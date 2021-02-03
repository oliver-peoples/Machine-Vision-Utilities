namespace mv_xsens
{
    class CallbackHandler : public XsCallback
    {
    public:
        CallbackHandler(size_t maxBufferSize = 5)
            : m_maxNumberOfPacketsInBuffer(maxBufferSize)
            , m_numberOfPacketsInBuffer(0)
        {
        }

        virtual ~CallbackHandler() throw()
        {
        }

        bool packetAvailable() const
        {
            xsens::Lock locky(&m_mutex);
            return m_numberOfPacketsInBuffer > 0;
        }

        XsDataPacket getNextPacket()
        {
            assert(packetAvailable());
            xsens::Lock locky(&m_mutex);
            XsDataPacket oldestPacket(m_packetBuffer.front());
            m_packetBuffer.pop_front();
            --m_numberOfPacketsInBuffer;
            return oldestPacket;
        }

    protected:
        virtual void onLiveDataAvailable(XsDevice*, const XsDataPacket* packet)
        {
            xsens::Lock locky(&m_mutex);
            assert(packet != nullptr);
            while (m_numberOfPacketsInBuffer >= m_maxNumberOfPacketsInBuffer)
                (void)getNextPacket();

            m_packetBuffer.push_back(*packet);
            ++m_numberOfPacketsInBuffer;
            assert(m_numberOfPacketsInBuffer <= m_maxNumberOfPacketsInBuffer);
        }
    private:
        mutable xsens::Mutex m_mutex;

        size_t m_maxNumberOfPacketsInBuffer;
        size_t m_numberOfPacketsInBuffer;
        std::list<XsDataPacket> m_packetBuffer;
    };

    auto handleError(std::string errorString, XsControl* control)
    {
        control->destruct();
        std::cout << errorString << std::endl;
        std::cout << "Press [ENTER] to continue." << std::endl;
        std::cin.get();
        return -1;
    }

    hmath::Quaternion hmathConvert(XsQuaternion& quaternion)
    {
        return hmath::Quaternion(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z());
    }

    hmath::Quaternion hmathConvert(XsQuaternion quaternion)
    {
        return hmath::Quaternion(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z());
    }

    hmath::Vector3 hmathConvert(XsVector& vector3)
    {
        return hmath::Vector3(vector3[0], vector3[1], vector3[2]);
    }

    hmath::Vector3 hmathConvert(XsVector vector3)
    {
        return hmath::Vector3(vector3[0], vector3[1], vector3[2]);
    }
}