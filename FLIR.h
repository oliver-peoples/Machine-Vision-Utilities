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
}