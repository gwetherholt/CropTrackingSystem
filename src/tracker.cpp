#include "tracker.hpp"
#include <cmath>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <iostream>

// Constructor definition with IoU threshold
Tracker::Tracker(int max_missing, float max_dist, float iou_threshold)
    : next_id_(0), max_missing_(max_missing), max_dist_(max_dist), iou_threshold_(iou_threshold) {}

// Helper function to compute Intersection over Union (IoU) between two bounding boxes
float Tracker::computeIoU(const TrackedObject& t, const Detection& d, int W, int H)
{
    // Convert normalized coordinates to pixel space
    float tx1 = t.x * W, ty1 = t.y * H;
    float tx2 = tx1 + t.width * W, ty2 = ty1 + t.height * H;
    float dx1 = d.x * W, dy1 = d.y * H;
    float dx2 = dx1 + d.width * W, dy2 = dy1 + d.height * H;

    // Compute the intersection coordinates
    float interX1 = std::max(tx1, dx1);
    float interY1 = std::max(ty1, dy1);
    float interX2 = std::min(tx2, dx2);
    float interY2 = std::min(ty2, dy2);

    // Compute the intersection area
    float interArea = std::max(0.f, interX2 - interX1) * std::max(0.f, interY2 - interY1);
    // Compute the union area
    float tArea = (tx2 - tx1) * (ty2 - ty1);
    float dArea = (dx2 - dx1) * (dy2 - dy1);
    float unionArea = tArea + dArea - interArea;

    return interArea / unionArea;  // Return IoU
}

// Update method to track objects
std::vector<TrackedObject> Tracker::update(const std::vector<Detection>& detections, int W, int H)
{
    std::vector<bool> matched(tracked_.size(), false);  // Keeps track of matched objects

    // Iterate over the detections
    for (const auto& d : detections)
    {
        float best_iou = 0.f;
        int idx = -1;
        bool matched_this_detection = false;

        // Try to match the current detection with existing tracked objects using IoU
        for (size_t i = 0; i < tracked_.size(); ++i)
        {
            if (matched[i]) continue;

            // IoU matching
            float iou = computeIoU(tracked_[i], d, W, H);
            if (iou > best_iou && iou >= iou_threshold_) {
                best_iou = iou;
                idx = static_cast<int>(i);
                matched_this_detection = true;
            }
        }

        // If no match by IoU, try matching by spatial proximity (Euclidean distance)
        if (!matched_this_detection)
        {
            float best_dist = max_dist_;
            for (size_t i = 0; i < tracked_.size(); ++i)
            {
                if (matched[i]) continue;

                float dx = tracked_[i].x - d.x;
                float dy = tracked_[i].y - d.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < best_dist) {
                    best_dist = dist;
                    idx = static_cast<int>(i);
                }
            }
        }

        // If matched, update the tracked object and its history
        if (idx != -1) {
            auto& t = tracked_[idx];
            t.x = d.x; t.y = d.y;
            t.width = d.width; t.height = d.height;
            t.last_seen = 0;
            t.history.push_back(cv::Point(int(d.x * W), int(d.y * H)));  // Add position to history (scaled to image dimensions)
            matched[idx] = true;
        } else {  // If no match, create a new tracked object
            TrackedObject t{ next_id_++, d.x, d.y, d.width, d.height, 0, {cv::Point(int(d.x * W), int(d.y * H))}};
            tracked_.push_back(t);
            matched.push_back(true);
        }
    }

    // Remove objects that have not been seen for too many frames
    for (size_t i = 0; i < tracked_.size(); ++i)
        if (!matched[i]) tracked_[i].last_seen++;

    tracked_.erase(std::remove_if(tracked_.begin(), tracked_.end(),
        [this](const TrackedObject& t) { return t.last_seen > max_missing_; }), tracked_.end());

    return tracked_;
}

// Visualize method to create the summary and frames
cv::Mat Tracker::visualize(const std::vector<TrackedObject>& objects, int frame_no, int W, int H) const
{
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(19, 69, 139)); // Background color (brown like soil)

    // Loop over each object in the tracked objects list
    for (const auto& o : objects)
    {
        bool px = (o.x > 1.f || o.y > 1.f || o.width > 1.f || o.height > 1.f);
        float cx = px ? o.x : o.x * W;
        float cy = px ? o.y : o.y * H;
        float ww = px ? o.width : o.width * W;
        float hh = px ? o.height : o.height * H;

        // Create a rectangle (bounding box) around the object
        cv::Rect r(int(cx - ww / 2), int(cy - hh / 2), int(ww), int(hh));

        // Draw the bounding box for the object (current position)
        cv::rectangle(img, r, cv::Scalar(0, 255, 0), 2); // Green for bounding box

        // Add the object ID label near the bounding box
        cv::putText(img, "ID:" + std::to_string(o.id),
                    {r.x, r.y - 5}, cv::FONT_HERSHEY_SIMPLEX,
                    0.5, cv::Scalar(0, 0, 255), 1); // Red text for ID

        // Draw the motion lines (full trajectory) for the object's history
        for (size_t i = 1; i < o.history.size(); ++i)
        {
            // Draw lines between all positions in the history (full path)
            cv::line(img, o.history[i-1], o.history[i], cv::Scalar(255, 0, 0), 2); // Blue motion lines
        }
    }

    // Add frame number label in the top left corner
    std::string label = (frame_no >= 0)
                        ? "Frame " + std::to_string(frame_no)
                        : "Summary";
    cv::putText(img, label, {10, 30}, cv::FONT_HERSHEY_SIMPLEX,
                1.0, cv::Scalar(0, 0, 0), 2); // Black label for frame number

    return img;
}
