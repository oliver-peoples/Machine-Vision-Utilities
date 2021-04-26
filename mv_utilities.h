#include <sstream>
#include <hmath.hpp>
#include <istream>
#include <fstream>
#include <ostream>

namespace mv
{
    const char quit_character = 'q';
    const char save = 's';

    namespace lenses
    {
        const std::string lens_EO86900 = "EO86900";
    }

    void naiveShow(cv::Mat& img, int delay = 10)
    {
        cv::imshow("Image", img);
        cv::waitKey(delay);
    }

    namespace utils
    {
        struct Corners
        {
            cv::Point2f tl, tr, bl, br;

            Corners(cv::Mat& source_img)
            {
                this->tl.x = 0;
                this->tl.y = 0;

                this->tr.x = source_img.size().width;
                this->tr.y = 0;

                this->bl.x = 0;
                this->bl.y = source_img.size().height;

                this->br.x = source_img.size().width;
                this->br.y = source_img.size().height;
            }

            Corners(cv::Point2f tl, cv::Point2f tr, cv::Point2f bl, cv::Point2f br)
            {
                this->tl = tl;
                this->tr = tr;
                this->bl = bl;
                this->br = br;
            }

            std::vector<cv::Point2f> getVector()
            {
                std::vector<cv::Point2f> corners;

                corners.push_back(this->tl);
                corners.push_back(this->tr);
                corners.push_back(this->bl);
                corners.push_back(this->br);

                return corners;
            }
        };
        
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
        
        calibration loadCalibration(std::string root)
        {
            std::vector<std::vector<double>> cameraMatrix_vector;
            std::vector<double> distCoeffs_vector;

            std::ifstream cameraMatrix_csv;
            std::ifstream distCoeffs_csv;

            std::string cameraMatrix_csv_path = root + "cameraMatrix.csv";
            std::string distCoeffs_csv_path = root + "distCoeffs.csv";

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

        bool d_approx_gte(double d1, double d2, int power=8)
        {
            int d1_rounded = round(d1 * pow(10, power));
            int d2_rounded = round(d2 * pow(10, power));

            return d1_rounded >= d2_rounded;
        }

        bool d_approx_lt(double d1, double d2, int power=8)
        {
            int d1_rounded = round(d1 * pow(10, power));
            int d2_rounded = round(d2 * pow(10, power));

            return d1_rounded < d2_rounded;
        }

        bool d_approx_gt(double d1, double d2, int power=8)
        {
            int d1_rounded = round(d1 * pow(10, power));
            int d2_rounded = round(d2 * pow(10, power));

            return d1_rounded > d2_rounded;
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

        template <typename v_T> hmath::Vector3<v_T> cvVec3dNormalized(cv::Vec3d v)
        {
            return hmath::Vector3<v_T>(v[0], v[1], v[2], hmath::NORMED);
        }

        template <typename v_T> hmath::Vector3<v_T> cvVec3d2hmath(cv::Vec3d v)
        {
            return hmath::Vector3<v_T>(v[0], v[1], v[2]);
        }

        template <typename q_T> hmath::Quaternion<q_T> getTTPrimeMappingQuaternion(hmath::Quaternion<q_T> T, hmath::Quaternion<q_T> TPrime)
        {
            return T.getInverse() * TPrime;
        }

        template <typename q_T> hmath::Quaternion<q_T> quaternionFromRVEC(cv::Vec3d rvec)
        {
            q_T theta = sqrt(rvec[0] * rvec[0] + rvec[1] * rvec[1] + rvec[2] * rvec[2]);
            hmath::Vector3<q_T> axis(rvec[0], rvec[1], rvec[2]);

            return hmath::Quaternion<q_T>(axis, theta);
        }

        template <typename q_T> cv::Vec3d quaternionToRVEC(hmath::Quaternion<q_T> q)
        {
            q.normalize();
            q_T angle = 2 * acos(q.w);
            q_T s = sqrt(1 - q.w * q.w);
            if (s < 0.00001)
            {
                return cv::Vec3d(q.i, q.j, q.k);
            }
            else
            {
                return cv::Vec3d(q.i / s, q.j / s, q.k / s);
            }
        }

        // hmath::DualQuaternion dualQuaternionFromRVEC_TVEC(cv::Vec3d rvec, cv::Vec3d tvec)
        // {
        //     hmath::Vector3 translation(tvec[0], tvec[1], tvec[2]);
        //     hmath::Quaternion rotation = quaternionFromRVEC(rvec);

        //     return hmath::DualQuaternion(rotation, translation);
        // }
    }
}