# Lessons-learned
This document describes about what lessons I learned from the project.

## Camera Pipeline
Camera pipeline captures the stream of images from camera into frames, 
stores the captured frames into buffers,
and let's the consumer consume the frame for processing.

### 1. System Design

#### 1) Baseline design 

* Data flow:
        frame               frame
Camera ----> Camera Capture ----> Process 

* Runtime system diagram

          



    





#### 2) camera pipeline 
