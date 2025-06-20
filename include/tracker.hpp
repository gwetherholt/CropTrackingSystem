#ifndef TRACKER_HPP
#define TRACKER_HPP

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
    // Updated constructor to include iou_threshold
    Tracker(int max_missing = 3, float max_dist = 0.1f, float iou_threshold = 0.5f);

    std::vector<TrackedObject> update(const std::vector<Detection>& detections, int W, int H);

    // W, H default match the visual canvas in main
    cv::Mat visualize(const std::vector<TrackedObject>& objects,
                      int frame_no, int W = 640, int H = 480) const;

private:
    int   next_id_;
    int   max_missing_;
    float max_dist_;
    float iou_threshold_;  // IoU threshold to determine valid object matches
    std::vector<TrackedObject> tracked_;

    // Helper method to compute Intersection over Union (IoU)
    float computeIoU(const TrackedObject& t, const Detection& d, int W = 640, int H = 480);
};

#endif // TRACKER_HPP
