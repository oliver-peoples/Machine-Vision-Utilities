#pragma once

namespace flir
{
    void PrintDeviceInfo(Spinnaker::GenApi::INodeMap& nodeMap)
    {
        int result = 0;
        std::cout << std::endl << "*** DEVICE INFORMATION ***" << std::endl << std::endl;

        try
        {
            Spinnaker::GenApi::FeatureList_t features;
            const Spinnaker::GenApi::CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
            if (IsAvailable(category) && IsReadable(category))
            {
                category->GetFeatures(features);

                for (auto it = features.begin(); it != features.end(); ++it)
                {
                    const Spinnaker::GenApi::CNodePtr pfeatureNode = *it;
                    std::cout << pfeatureNode->GetName() << " : ";
                    Spinnaker::GenApi::CValuePtr pValue = static_cast<Spinnaker::GenApi::CValuePtr>(pfeatureNode);
                    std::cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
                    std::cout << std::endl;
                }
            }
            else
            {
                std::cout << "Device control information not available." << std::endl;
            }
        }
        catch (Spinnaker::Exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
            result = -1;
        }
    }

    void getImage(cv::Mat& mat, Spinnaker::CameraPtr pCam)
    {
        Spinnaker::ImagePtr pResultImage = pCam->GetNextImage(1000);

        unsigned int rows = pResultImage->GetHeight();
        unsigned int cols = pResultImage->GetWidth();
        unsigned int num_channels = pResultImage->GetNumChannels();
        void *image_data = pResultImage->GetData();
        unsigned int stride = pResultImage->GetStride();

        mat = cv::Mat(rows, cols, (num_channels == 3) ? CV_8UC3 : CV_8UC1, image_data, stride);
    }

    cv::Mat getImage(Spinnaker::CameraPtr pCam)
    {
        Spinnaker::ImagePtr pResultImage = pCam->GetNextImage(1000);

        unsigned int rows = pResultImage->GetHeight();
        unsigned int cols = pResultImage->GetWidth();
        unsigned int num_channels = pResultImage->GetNumChannels();
        void *image_data = pResultImage->GetData();
        unsigned int stride = pResultImage->GetStride();
        
        return cv::Mat(rows, cols, (num_channels == 3) ? CV_8UC3 : CV_8UC1, image_data, stride);
    }

    namespace spinnaker
    {
        void standardSetup_ManualBuffer(Spinnaker::CameraPtr& pCam, int buffer_size=5)
        {
            Spinnaker::GenApi::INodeMap& nodeMap = pCam->GetNodeMap();

            Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
            if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
            {
                std::cout << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            Spinnaker::GenApi::CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
            if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
            {
                std::cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

            ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

            Spinnaker::GenApi::CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
            if (!Spinnaker::GenApi::IsAvailable(ptrTriggerMode) || !Spinnaker::GenApi::IsReadable(ptrTriggerMode))
            {
                std::cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            Spinnaker::GenApi::CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
            if (!Spinnaker::GenApi::IsAvailable(ptrTriggerModeOff) || !Spinnaker::GenApi::IsReadable(ptrTriggerModeOff))
            {
                std::cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());

            Spinnaker::GenApi::INodeMap& sNodeMap = pCam->GetTLStreamNodeMap();

            Spinnaker::GenApi::CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
            if (!Spinnaker::GenApi::IsAvailable(ptrHandlingMode) || !Spinnaker::GenApi::IsWritable(ptrHandlingMode))
            {
                std::cout << "Unable to get stream buffer handling mode. Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            Spinnaker::GenApi::CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
            if (!Spinnaker::GenApi::IsAvailable(ptrHandlingModeEntry) || !Spinnaker::GenApi::IsReadable(ptrHandlingModeEntry))
            {
                std::cout << "Unable to get current stream buffer handling mode. Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            // Set stream buffer Count Mode to manual
            Spinnaker::GenApi::CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode("StreamBufferCountMode");
            if (!Spinnaker::GenApi::IsAvailable(ptrStreamBufferCountMode) || !Spinnaker::GenApi::IsWritable(ptrStreamBufferCountMode))
            {
                std::cout << "Unable to set Buffer Count Mode (node retrieval). Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            Spinnaker::GenApi::CEnumEntryPtr ptrStreamBufferCountModeManual = ptrStreamBufferCountMode->GetEntryByName("Manual");
            if (!Spinnaker::GenApi::IsAvailable(ptrStreamBufferCountModeManual) || !Spinnaker::GenApi::IsReadable(ptrStreamBufferCountModeManual))
            {
                std::cout << "Unable to set Buffer Count Mode entry (Entry retrieval). Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            ptrStreamBufferCountMode->SetIntValue(ptrStreamBufferCountModeManual->GetValue());

            // Retrieve and modify Stream Buffer Count
            Spinnaker::GenApi::CIntegerPtr ptrBufferCount = sNodeMap.GetNode("StreamBufferCountManual");
            if (!Spinnaker::GenApi::IsAvailable(ptrBufferCount) || !Spinnaker::GenApi::IsWritable(ptrBufferCount))
            {
                std::cout << "Unable to set Buffer Count (Integer node retrieval). Aborting..." << std::endl << std::endl;
                exit(EXIT_FAILURE);
            }

            ptrBufferCount->SetValue(buffer_size);

            ptrHandlingModeEntry = ptrHandlingMode->GetEntryByName("NewestOnly");
            ptrHandlingMode->SetIntValue(ptrHandlingModeEntry->GetValue());
        }

        std::string getModel(Spinnaker::GenApi::INodeMap& nodeMapTLDevice)
        {
            Spinnaker::GenICam::gcstring deviceModel("");
            Spinnaker::GenApi::CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceModelName");
            deviceModel = ptrStringSerial->GetValue();

            std::string deviceModelString = deviceModel.c_str();

            std::string::iterator end_pos = std::remove(deviceModelString.begin(), deviceModelString.end(), ' ');
            deviceModelString.erase(end_pos, deviceModelString.end());

            return deviceModelString;
        }

        std::string getId(Spinnaker::GenApi::INodeMap& nodeMapTLDevice)
        {
            Spinnaker::GenICam::gcstring deviceID("");
            Spinnaker::GenApi::CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
            deviceID = ptrStringSerial->GetValue();

            std::string id = std::string(deviceID.c_str()).substr(0,2);
            id += std::string(deviceID.c_str()).substr(std::string(deviceID.c_str()).length() - 2);

            return id;
        }

        cv::Size getResolution(Spinnaker::CameraPtr& pCam)
        {
            Spinnaker::GenApi::CIntegerPtr ptrIntegerWidth = pCam->GetNodeMap().GetNode("Width");
            Spinnaker::GenApi::CIntegerPtr ptrIntegerHeight = pCam->GetNodeMap().GetNode("Height");

            int64_t image_width = ptrIntegerWidth->GetValue();
            int64_t image_height = ptrIntegerHeight->GetValue();

            return cv::Size(image_width, image_height);
        }
    }
}