#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

struct Detection {
    float x, y, width, height;
};

struct TrackedObject {
    int   id;
    float x, y, width, height;
    int   last_seen;
    std::vector<cv::Point> history;
};

class Tracker {
public:
    Tracker(int max_missing = 3, float max_dist = 0.1f);

    std::vector<TrackedObject> update(const std::vector<Detection>& detections);

    // W,H default match the visual canvas in main
    cv::Mat visualize(const std::vector<TrackedObject>& objects,
                      int frame_no, int W = 640, int H = 480) const;

private:
    int   next_id_;
    int   max_missing_;
    float max_dist_;
    std::vector<TrackedObject> tracked_;
};
