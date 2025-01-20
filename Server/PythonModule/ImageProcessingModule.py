import cv2
import numpy as np

class OPTICALFLOW:
    def __init__(self):
        self.lk_params = dict(winSize=(15, 15),
                              maxLevel=2,
                              criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))
        self.feature_params = dict(maxCorners=100,
                                   qualityLevel=0.3,
                                   minDistance=7,
                                   blockSize=7)
        self.features = {}  # Dictionary to store the positions of features across frames
        self.prev_img = None
        self.prev_pts = None
        self.frame_number = 0
        self.min_features = 8  # Minimum number of features threshold

    def detect_initial_features(self, frame):
        if isinstance(frame, np.ndarray) and frame.ndim == 3 and frame.shape[2] == 3:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            p0 = cv2.goodFeaturesToTrack(gray, mask=None, **self.feature_params)
            if p0 is not None:
                for i, pt in enumerate(p0):
                    self.features[i] = [(self.frame_number, pt[0])]
                self.prev_pts = p0
                self.prev_img = gray

    def add_new_features(self, frame):
        if isinstance(frame, np.ndarray) and frame.ndim == 3 and frame.shape[2] == 3:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            p0 = cv2.goodFeaturesToTrack(gray, mask=None, **self.feature_params)
            if p0 is not None:
                new_pts = []
                for i, pt in enumerate(p0):
                    if len(self.features) < self.feature_params['maxCorners']:
                        feature_id = len(self.features) + i
                        self.features[feature_id] = [(self.frame_number, pt[0])]
                        new_pts.append(pt)
                if new_pts:
                    self.prev_pts = np.vstack((self.prev_pts, np.array(new_pts)))

    def track_features(self, frame):
    
            if isinstance(frame, np.ndarray) and frame.ndim == 3 and frame.shape[2] == 3:
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                if self.prev_pts is not None and self.prev_img is not None:
                    p1, st, err = cv2.calcOpticalFlowPyrLK(self.prev_img, gray, self.prev_pts, None, **self.lk_params)
                    good_new = p1[st == 1]
                    good_old = self.prev_pts[st == 1]

                    for i, (new, old) in enumerate(zip(good_new, good_old)):
                        a, b = new.ravel()
                        c, d = old.ravel()
                        feature_id = list(self.features.keys())[i]
                        self.features[feature_id].append((self.frame_number, (a, b)))

                    # Remove features that are no longer visible
                    if len(good_old) < len(self.prev_pts):
                        lost_features = [list(self.features.keys())[i] for i in range(len(good_old), len(self.features))]
                        for feature_id in lost_features:
                            print(f"Feature ID {feature_id} no longer visible.")
                            for frame, pos in self.features[feature_id]:
                                print(f"  Frame {frame}: Position {pos}")
                            del self.features[feature_id]

                    self.prev_pts = good_new.reshape(-1, 1, 2)
                    self.prev_img = gray

                    # Add new features if the number of tracked points drops below the threshold
                    if len(self.prev_pts) < self.min_features:
                        self.add_new_features(frame)
        

    def draw_features(self, frame):
        if self.prev_pts is not None:
            for pt in self.prev_pts:
                x, y = pt.ravel()
                cv2.circle(frame, (int(x), int(y)), 5, (0, 255, 0), -1)
        return frame

