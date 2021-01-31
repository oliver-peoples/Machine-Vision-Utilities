

namespace mv
{
    void naiveShow(cv::Mat& img, int delay = 10)
    {
        cv::imshow("Image", img);
        cv::waitKey(delay);
    }

    namespace calibration
    {

        class calibration
        {
        private:
            /* data */
        public:
            cv::Mat cameraMatrix = cv::Mat(3, 3, CV_64F);
            cv::Mat distCoeffs;

            calibration()
            {
                this->distCoeffs = cv::Mat(1, 5, CV_64F);
            }
            ~calibration() {}

            calibration(int distCoeffsWidth)
            {
                this->distCoeffs = cv::Mat(1,distCoeffsWidth, CV_64F);
            }
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
        
        calibration loadCalibration(std::string root, std::string model, std::string id, std::string lens)
        {
            std::vector<std::vector<double>> cameraMatrix_vector;
            std::vector<double> distCoeffs_vector;

            std::ifstream cameraMatrix_csv;
            std::ifstream distCoeffs_csv;

            std::string cameraMatrix_csv_path = root + "/" + model + "-" + id + "/" + lens + "/cameraMatrix.csv";
            std::string distCoeffs_csv_path = root + "/" + model + "-" + id + "/" + lens + "/distCoeffs.csv";

            cameraMatrix_csv.open(cameraMatrix_csv_path);
            distCoeffs_csv.open(distCoeffs_csv_path);   

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