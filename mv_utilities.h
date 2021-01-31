

namespace mv
{
    namespace cvTools
    {
        void naiveShow(cv::Mat& img, int delay = 10)
        {
            cv::imshow("Image", img);
            cv::waitKey(delay);
        }
    }

    namespace spinnaker
    {
        void standardSetup(Spinnaker::CameraPtr pCam)
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

            ptrBufferCount->SetValue(5);

            ptrHandlingModeEntry = ptrHandlingMode->GetEntryByName("NewestOnly");
            ptrHandlingMode->SetIntValue(ptrHandlingModeEntry->GetValue());
        }
    }

    namespace calibration
    {
        struct calibration
        {
            cv::Mat cameraMatrix = cv::Mat(3, 3, CV_64F);
            cv::Mat distCoeffs = cv::Mat(1, 5, CV_64F);
        };

        cv::Mat idealDistCoeffs = cv::Mat::zeros(1, 5, CV_64F);

        cv::Mat getIdealCameraMatrix(cv::Mat& cameraMatrix, cv::Size imgSize)
        {
            cv::Mat ideal = cv::Mat::eye(3, 3, CV_64F);
            
            double average_focal_length = (cameraMatrix.at<double>(0,0) + cameraMatrix.at<double>(1,1)) / 2;

            ideal.at<double>(0,0) = average_focal_length;
            ideal.at<double>(1,1) = average_focal_length;

            ideal.at<double>(0,2) = imgSize.width / 2;
            ideal.at<double>(1,2) = imgSize.height / 2;

            return ideal;
        }
        
        calibration loadCalibration(std::string sn = "")
        {
            std::vector<std::vector<double>> cameraMatrix_vector;
            std::vector<double> distCoeffs_vector;

            std::ifstream cameraMatrix_csv;
            std::ifstream distCoeffs_csv;

            if (sn == "")
            {
                cameraMatrix_csv.open("/home/finn/camera_calibrations/cameraMatrix.csv");
                distCoeffs_csv.open("/home/finn/camera_calibrations/distCoeffs.csv");   
            }

            std::string cameraMatrix_row, cameraMatrix_column;
            
            while (std::getline(cameraMatrix_csv, cameraMatrix_row))
            {
                std::stringstream ss(cameraMatrix_row);

                std::vector<double> row_vals;

                while (std::getline(ss, cameraMatrix_column, ','))
                {
                    row_vals.push_back(std::stod(cameraMatrix_column));
                }
                
                cameraMatrix_vector.push_back(row_vals);
            }
            
            std::string distCoeffs_row, distCoeffs_column;

            std::getline(distCoeffs_csv, distCoeffs_row);

            std::stringstream ss(distCoeffs_row);

            while (std::getline(ss, distCoeffs_column, ','))
            {
                distCoeffs_vector.push_back(std::stod(distCoeffs_column));
            }
            
            calibration cal;

            for (int j = 0; j < 3; j++)
            {
                for (int i = 0; i < 3; i++)
                {
                    cal.cameraMatrix.at<double>(j,i) = cameraMatrix_vector[j][i];
                }
                
            }

            for (int i = 0; i < 5; i++)
            {
                cal.distCoeffs.at<double>(0,i) = distCoeffs_vector[i];
            }
            
            return cal;
        }
    }

    namespace math
    {
        bool d_approx_et(double d1, double d2, int power=8)
        {
            int d1_rounded = round(d1 * pow(10, power));
            int d2_rounded = round(d2 * pow(10, power));

            return d1_rounded == d2_rounded;
        }

        bool d_approx_lte(double d1, double d2, int power=8)
        {
            int d1_rounded = round(d1 * pow(10, power));
            int d2_rounded = round(d2 * pow(10, power));

            return d1_rounded <= d2_rounded;
        }

        hmath::Vector3 h_xAxis(1,0,0);
        hmath::Vector3 h_yAxis(0,1,0);
        hmath::Vector3 h_zAxis(0,0,1);
        
        cv::Vec3d origin_rvec = {0,0,0};
        cv::Vec3d origin_tvec = {0,0,0};

        double cvVec3dNorm(cv::Vec3d v)
        {
            return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        }

        hmath::Vector3 cvVec3dNormalized(cv::Vec3d v)
        {
            return hmath::Vector3(v[0], v[1], v[2], hmath::NORMED);
        }

        hmath::Vector3 cvVec3d2hmath(cv::Vec3d v)
        {
            return hmath::Vector3(v[0], v[1], v[2]);
        }

        hmath::Quaternion getTTPrimeMappingQuaternion(hmath::Quaternion T, hmath::Quaternion TPrime)
        {
            return T.getInverse() * TPrime;
        }

        hmath::Quaternion quaternionFromRVEC(cv::Vec3d rvec)
        {
            double thetaOn2 = cvVec3dNorm(rvec) / 2;
            hmath::Vector3 rvec_normed = cvVec3dNormalized(rvec);
            

            return hmath::Quaternion(cos(thetaOn2), sin(thetaOn2) * rvec_normed.i, sin(thetaOn2) * rvec_normed.j, sin(thetaOn2) * rvec_normed.k);
        }

        cv::Vec3d quaternionToRVEC(hmath::Quaternion q)
        {
            hmath::Vector3 axis = q.getVectorComponent();
            axis.normalize();
            axis *= (2 * acos(q.w));

            return cv::Vec3d(axis.i, axis.j, axis.k);
        }

        hmath::DualQuaternion dualQuaternionFromRVEC_TVEC(cv::Vec3d rvec, cv::Vec3d tvec)
        {
            hmath::Vector3 translation(tvec[0], tvec[1], tvec[2]);
            hmath::Quaternion rotation = quaternionFromRVEC(rvec);

            return hmath::DualQuaternion(rotation, translation);
        }
    }
}