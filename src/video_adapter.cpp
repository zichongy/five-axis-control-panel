#include "video_adapter.h"
#include "ui_video_adapter.h"

#include "moc_video_adapter.cpp"

OpenCVAdapter::OpenCVAdapter(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

OpenCVAdapter::~OpenCVAdapter()
{
    flag_camera_stop_ = true;
}

void OpenCVAdapter::run(){
    cv::VideoCapture cv_capture_;
    cv::Mat frame;
    bool camera_open_success_ = cv_capture_.open(camera_index_); //从指定摄像头读入
    qDebug() << "Camera open success: " << camera_open_success_;

    if (camera_open_success_)
    {
        //设置并获取相机分辨率和帧率
        cv_capture_.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cv_capture_.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        int width = (int) cv_capture_.get(cv::CAP_PROP_FRAME_WIDTH);
        int height = (int) cv_capture_.get(cv::CAP_PROP_FRAME_HEIGHT);
        int rate = (int) cv_capture_.get(cv::CAP_PROP_FPS);
        qDebug() << "Camera has the resolution of " << width << " x " << height
                 << " with a frame rate of " << rate << " FPS.";
        //循环送帧
        while (!flag_camera_stop_)
        {
            cv_capture_ >> frame;
            if(frame.empty())
            {
                qDebug() << "OpenCVAdapter: 解析失败: 帧为空。";
                continue;
            }
            else if(!frame.empty())
            {
                emit sigSendFrame(frame);
            }
        }
    }
}

void OpenCVAdapter::cvFrameInitialize(bool is_camera_perm_,int desired_camera_index_){
    flag_camera_stop_ = true;
    wait();
    flag_camera_stop_ = !is_camera_perm_;
    qDebug() << "Camera is stop: " << flag_camera_stop_;
    camera_index_ = desired_camera_index_;
    if (!flag_camera_stop_) start();
}

VideoAdapter::VideoAdapter(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoAdapter)
{
    //User Interface
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(1);

    //Camera
    m_thread = new OpenCVAdapter(this);
    connect(m_thread, SIGNAL(sigSendFrame(cv::Mat)),this,SLOT(slotGetFrame(cv::Mat)));
    connect(this, SIGNAL(sigCVInitialize(bool,int)),m_thread,SLOT(cvFrameInitialize(bool,int)));

    //Frame Analyze
    //Load the aruco_dictionary_ that was used to generate the markers.
    aruco_dictionary_ = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    //Initialize the detector aruco_parameters_ using default values
    aruco_parameters_ = cv::aruco::DetectorParameters::create();
}

VideoAdapter::~VideoAdapter(){
    emit sigCVInitialize(false,0);//发送信号关闭相机，结束线程
    if(m_thread != nullptr){
        m_thread->wait();
        delete m_thread;
    }
    delete ui;
}

void VideoAdapter::cameraStatusChanged(bool camera_permission, int desired_camera_index){
    flag_camera_open_ = camera_permission;
    emit sigCVInitialize(camera_permission,desired_camera_index);
    frame_countdown_ = frame_interval_;

    if(flag_camera_open_) ui->stackedWidget->setCurrentIndex(0);
    else ui->stackedWidget->setCurrentIndex(1);
}

void VideoAdapter::slotGetFrame(cv::Mat mat_src_){
    //隔frame_interval_帧处理一次
    if (frame_countdown_ == frame_interval_){
        frame_countdown_ -= 1;
        mat_src_ = framePosAnalyze(mat_src_,true);
    }
    else if (frame_countdown_ > 1){
        frame_countdown_ -= 1;
        mat_src_ = framePosAnalyze(mat_src_,false);
    }
    else{
        frame_countdown_ = frame_interval_;
        mat_src_ = framePosAnalyze(mat_src_,false);
    }

    //送去显示
    QImage src_image_ = cvMat2QImage(mat_src_);
    QPixmap original_pixmap_ = QPixmap::fromImage(src_image_);
    QPixmap scaled_pixmap_ = original_pixmap_.scaled(ui->label->width(),ui->label->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->label->setPixmap(scaled_pixmap_);
}

QImage VideoAdapter::cvMat2QImage(const cv::Mat &mat){
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols,mat.rows,QImage::Format_Indexed8);
        image.setColorCount(256);
        for (int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i,i,i));
        }

        uchar *pSrc = mat.data;
        for (int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    else if (mat.type() == CV_8UC3)
    {
        const auto *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, (int) mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
        //8位，通道数为4
    else if (mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";

        const auto *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, (int) mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
        //32位，通道数为3
    else if (mat.type() == CV_32FC3)
    {
        cv::Mat temp;
        mat.convertTo(temp, CV_8UC3);

        const auto *pSrc = (const uchar*)temp.data;
        QImage image(pSrc, temp.cols, temp.rows, (int) temp.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

void VideoAdapter::slotLaserTrackFlag(bool laserTrack_on_) {
    flag_laser_track_ = laserTrack_on_;
}

void VideoAdapter::slotAutoTraceFlag(bool autoTrace_on_) {
    emit sigAutoConSerialWrite(0,0);
    flag_auto_trace_ = autoTrace_on_;
}

void VideoAdapter::slotRotateAngle(double angle_deg_) {
    rotate_angle_ = - (angle_deg_ * M_PI) / 180;
}

cv::Mat VideoAdapter::framePosAnalyze(cv::Mat frame_,bool is_key_frame_)
{
    cv::Mat frame_copy_ = frame_;

    //是关键帧,则刷新检测
    if(is_key_frame_){
        cv::aruco::detectMarkers(frame_,aruco_dictionary_,markerCorners,markerIds,
                                  aruco_parameters_, rejectedCandidates);

        //如果有检测到，计算目标点位置
        if(!markerIds.empty()) {
            dest_point_ = findDestPoint(markerCorners, markerIds);
            //如果能计算目标点位置 且 激光点检测打开，则进一步计算激光点位置并进行位置规划
            if (dest_point_ != cv::Point2f(0,0) && flag_laser_track_){
                laser_point_ = findLaserPoint(frame_copy_);
                if(flag_auto_trace_) {
                    in_thresh_ = diffInThresh(dest_point_,laser_point_);
                    if (in_thresh_) {
                        flag_x_stat_ = 0;
                        flag_y_stat_ = 0;
                        flag_auto_trace_ = false;
                        emit sigStopAutoTrace();
                        emit sigAutoConSerialWrite(flag_x_stat_,flag_y_stat_);
                    }
                    else {
                        autoMotionPlanning(dest_point_,laser_point_);
                    }
                }
            }
        }
    }

    //如果有检测到的ArUco标记和中心，将其画出，否则直接返回原图
    if(!markerIds.empty()){
        //绘制Marker
        cv::aruco::drawDetectedMarkers(frame_copy_,markerCorners,markerIds);

        //如果能获得目标点，绘制目标点
        if (dest_point_ != cv::Point2f(0,0)) {
            if (!flag_laser_track_) {
                cv::circle(frame_copy_, dest_point_, 5, cv::Scalar(0, 255, 255), 3);
                cv::putText(frame_copy_, "Destination", dest_point_, cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 1,cv::Scalar(0, 255, 255));
            }
            if(flag_laser_track_){
                cv::circle(frame_copy_,laser_point_,3,cv::Scalar(0,0,255),3);
                if(in_thresh_) cv::circle(frame_copy_,dest_point_,pixel_threshold,cv::Scalar(0,255,0),3);
                else cv::circle(frame_copy_,dest_point_,pixel_threshold,cv::Scalar(0,0,255),3);
            }
        }
        return frame_copy_;
    }
    else return frame_;
}

cv::Point2f VideoAdapter::findDestPoint(std::vector<std::vector<cv::Point2f>> _corners, std::vector<int> _ids) {
    int id_21, id_25, id_30, id_33;
    cv::Point2f p_21, p_25, p_30, p_33, p_middle;

//    int nMarkers = (int) _ids.size();
//    qDebug() << nMarkers << ":" << _ids;

    id_21 = (int) (std::find(_ids.begin(),_ids.end(),21) - _ids.begin());
    id_25 = (int) (std::find(_ids.begin(),_ids.end(),25) - _ids.begin());
    id_30 = (int) (std::find(_ids.begin(),_ids.end(),30) - _ids.begin());
    id_33 = (int) (std::find(_ids.begin(),_ids.end(),33) - _ids.begin());

    std::vector<bool> markerStatus({_ids[id_21] == 21, _ids[id_25] == 25, _ids[id_30] == 30, _ids[id_33] == 33});

    //获取各个Corner坐标
    if (markerStatus[0]) p_21 = _corners[id_21][0];
    if (markerStatus[1]) p_25 = _corners[id_25][0];
    if (markerStatus[2]) p_30 = _corners[id_30][0];
    if (markerStatus[3]) p_33 = _corners[id_33][0];

    if (markerStatus[0] && markerStatus[1] && markerStatus[2] && markerStatus[3]){
        p_middle = (p_21 + p_25 + p_30 + p_33) * 0.25;
        return  p_middle;
    }
    else if (markerStatus[0] && markerStatus[3]){
        p_middle = (p_21 + p_33) * 0.5;
        return  p_middle;
    }
    else if (markerStatus[1] && markerStatus[2]){
        p_middle = (p_25 + p_30) * 0.5;
        return  p_middle;
    }
    else{
        qDebug() << "One or several markers detected, but not enough to calculate the central point!";
        return {0,0};
    }
}

cv::Point2f VideoAdapter::findLaserPoint(cv::Mat &_img) {
    cv::Mat gray;
    cv::Point maxLoc;
    cv::cvtColor(_img,gray,cv::COLOR_BGR2GRAY);
    //cv::GaussianBlur(gray,gray,{51,51},0);
    cv::blur(gray,gray,{51,51});

    cv::minMaxLoc(gray, nullptr,nullptr,nullptr,&maxLoc);

    cv::Point2f maxLocfloat = maxLoc;
    return maxLocfloat;
}

void VideoAdapter::autoMotionPlanning(cv::Point2f &_dest, cv::Point2f &_current) {
    int x_stat;
    int y_stat;

    cv::Point2f diffVector = _dest - _current;
    cv::Point2f diffVector_prime = { (float) (diffVector.x * std::cos(rotate_angle_) - diffVector.y * std::sin(rotate_angle_)),
                                     (float) (diffVector.y * std::cos(rotate_angle_) + diffVector.x * std::sin(rotate_angle_))};

    //qDebug() << rotate_angle_ << diffVector.x << diffVector.y << diffVector_prime.x << diffVector_prime.y;

    if(abs(diffVector_prime.x) < pixel_threshold) {
        x_stat = 0;
    }
    else {
        if(diffVector_prime.x < 0){
            x_stat = 1;
        }
        else x_stat = 2;
    }

    if(abs(diffVector_prime.y) < pixel_threshold) {
        y_stat = 0;
    }
    else {
        if(diffVector_prime.y < 0){
            y_stat = 1;
        }
        else y_stat = 2;
    }

    if (x_stat != flag_x_stat_ || y_stat != flag_y_stat_){
        emit sigAutoConSerialWrite(x_stat,y_stat);
    }
}

bool VideoAdapter::diffInThresh(cv::Point2f &_dest, cv::Point2f &_current) {
    //计算两点Pix距离
    double diffPix = cv::norm(_dest - _current);

    //计算Pix和cm的换算系数
    int nMarker = (int) markerIds.size();
    double totalPixLen = 0.0;

    for(int i = 0; i < nMarker; i++){
        for(int j = 0; j < 4; j++){
            totalPixLen += cv::norm(markerCorners[i][j] - markerCorners[i][(j+1)%4]);
        }
    }

    double diffCenti = diffPix / (totalPixLen / (nMarker * 4 * 2));
    pixel_threshold = (int) (centimeter_threshold * (totalPixLen / (nMarker * 4 * 2)));

    //qDebug() << (totalPixLen / (nMarker * 4 * 2));

    if(diffCenti > centimeter_threshold) return false;
    else return true;
}