# Three Month Robotics Runtime Project Plan

**Project:** Webcam to Jetson Orin Nano to SO-101 object tracking  
**Owner:** Bruce Kim  
**Duration:** 12 development weeks, with any remaining days in the three-month window reserved for recovery and packaging  
**Effort assumption:** 10–12 hours per week, approximately 120–144 hours total  
**Planning date:** October 23, 2026

## 1. Outcome and engineering purpose

Build a small robot that tracks a moving object while producing defensible measurements of its camera, inference, and actuator pipeline. The main deliverable is a reproducible C++ runtime experiment with a physical demonstration.

The project should connect Bruce's existing accelerator-runtime experience to continuous robotics workloads: fresh observations, bounded memory, asynchronous execution, control-loop scheduling, USB/serial interfaces, and predictable behavior during failures.

The application should remain simple. Do not spend the project training a VLA, building a general manipulation stack, or maximizing detector accuracy. Use one existing lightweight detector and one target object.

By the end, a reviewer should be able to answer:

1. Where does an observation spend time before it becomes a robot command?
2. When processing falls behind, does the robot use old observations or discard them?
3. Which changes improve tail latency, and what do they cost in throughput, memory, power, and tracking quality?
4. What happens when a camera, inference worker, or actuator connection fails?
5. Can another engineer reproduce the results from the repository?

This is a soft real-time user-space project. Its measurements do not establish hard real-time guarantees or production safety certification.

## 2. Scope and assumptions

### Hardware and behavior

| Component | Planned configuration | Verify before relying on it |
|---|---|---|
| Compute | NVIDIA Jetson Orin Nano; record module RAM and carrier model | This is the Orin Nano, not the older Jetson Nano |
| Camera | USB webcam, nominally 60 FPS; start with a supported low-resolution mode | Actual resolution, pixel format, exposure, USB speed, and sustained delivered rate |
| Robot | SO-101 follower arm over USB/serial, powered by its appropriate supply | Calibration, limits, communication stability, and command/readback timing |
| Target | One easily recognized object; a colored marker for early bring-up | A chosen pretrained detector must recognize the final target without training |
| Motion | Limited pointing or camera-centering behavior using at most two joints | No grasping; remaining joints stay in a conservative calibrated pose |
| Runtime | C++ capture, processing, profiling, and scheduling | Initial robot integration may use a small LeRobot Python bridge |
| Software | A working JetPack stack, pinned OpenCV and LeRobot versions | Use APIs and TensorRT versions compatible with the installed JetPack |

**Primary tracking arrangement:** Mount a lightweight webcam near the wrist so the robot can turn it toward the target. Use small base-yaw and wrist-pitch position increments to center the target in the image. Verify camera mass, balance, mount security, and cable slack first.

**Fallback:** If the camera mount causes mechanical or cable problems, use a fixed camera and a calibrated one-dimensional target-position-to-base-yaw mapping. Label this as pointing behavior rather than camera-centering. Freeze the arrangement in Week 1; a weaker mechanical setup should not consume the runtime schedule.

Start with a colored-target detector to validate the pipeline. Introduce a small pretrained object detector for the actual inference baseline. Keep those results separate: replacing a neural detector with color thresholding is a change in workload, not an inference optimization.

### Explicit limits

- One camera, one target, one arm, and one local host.
- No depth reconstruction, arbitrary 3D reaching, object grasping, SLAM, or learned control.
- No new model training, custom CUDA kernels, or INT8 calibration in the required scope.
- No ROS 2 migration unless it solves a demonstrated integration problem.
- No kernel patches, PREEMPT_RT migration, or real-time scheduler changes in the required scope.
- No mandatory direct C++ servo protocol implementation. The bridge is acceptable if measured and bounded.
- No network transport in the required scope. This project addresses local pipelines and timing; it will not prove expertise in WebRTC or high-performance networking.

## 3. Work from the existing structure

The supplied filenames are the starting point. This plan proposes responsibilities; it does not assume their current implementations have been inspected or are complete.

| Existing file | Responsibility to establish |
|---|---|
| `camera_capture_baseline.cpp` | Blocking capture baseline; negotiated camera settings; frame IDs; capture-return timestamps |
| `latest_frame_mailbox.cpp` | Latest-frame exchange; replacement counters; ownership and shutdown semantics |
| `main.cpp` | Configuration, startup ordering, signal handling, coordinated stop |
| `opencv_process_infer.cpp` | Preprocessing and detector backends with separately measured stages |
| `utils/camera_utils.cpp` | Camera capability reporting and backend configuration |
| `utils/profiler.cpp` | Structured events, bounded logging, summaries, and run metadata |

Add modules only when their responsibilities become necessary:

- `frame_pool`: reusable frame storage with explicit ownership.
- `controller`: target selection, image error, joint increments, and command freshness checks.
- `robot_interface`: command/feedback contract and timing; initially talks to a LeRobot bridge.
- `watchdog`: fault state transitions and stale-command handling.
- `benchmark_runner`: headless runs, replay, load injection, and configuration capture.
- An analysis script to turn event logs into percentile tables and plots.

Keep the synchronous baseline available as a selectable mode. Do not overwrite it while implementing the optimized pipeline.

## 4. Runtime design

### Baseline

A synchronous loop provides a clear reference:

1. Read a frame.
2. Preprocess and run detection.
3. Select the target.
4. Compute a bounded joint target.
5. Send the command, when robot control is enabled.
6. Record timings.

First run without the robot, then with it. Capture-only, perception-only, and integrated results must be distinguishable.

### Optimized architecture

| Execution context | Job | Exchange policy |
|---|---|---|
| Capture thread | Continuously receive webcam frames and publish ownership handles | Latest-frame mailbox; capacity one |
| Inference worker | Take the newest available frame, preprocess, infer, postprocess | One inference initially in flight; publish latest completed observation |
| Control thread | Run on an absolute periodic schedule; validate freshness; compute bounded commands | Use latest valid observation; never wait for inference |
| Robot bridge | Send commands and read joint feedback | Latest command, bounded transport, independent command-expiry check |
| Logging worker | Drain timing records and write files | Bounded event buffer; count log loss |

Start with one mutex and condition variable where appropriate. A correct, measurable design matters more than a lock-free implementation.

Each observation carries the source frame ID and timestamps. Each command carries an observation ID, command ID, and expiration information. Reject out-of-order observations and expired commands. Reading the same observation on two control ticks must not be counted as two new inference results.

A frame slot is reusable only after its final reader releases it. A shallow `cv::Mat` copy does not create independent pixel storage. Capture must never overwrite storage still being read by preprocessing or inference.

Keep intentional application-level frame replacement separate from camera/driver drops and inference failures.

### Minimal control rule

Use normalized image-center error, a deadband, and a small proportional correction. Calibrate the sign and sensitivity of each active joint with small supervised moves.

- Constrain absolute joint position and per-tick position change.
- Use joint feedback to detect communication problems and command saturation.
- Start with one active joint, then enable the second if the first is stable.
- If joint coupling is significant, estimate a local two-by-two image Jacobian rather than tuning blindly.
- Record smoothing parameters; smoothing can reduce noise while increasing reaction delay.
- Do not describe a successful serial write as completed physical motion.

Use a fixed-size command message across the C++/Python boundary. A local Unix-domain socket is sufficient. Include bridge receive, send, and feedback timestamps so transport and Python scheduling costs remain visible.

## 5. Measurement contract

### Timestamp definitions

Use one explicitly selected monotonic host clock for comparable CPU events across C++ and Python, such as `CLOCK_MONOTONIC` in both processes. Record clock IDs. Do not subtract timestamps from unknown or incompatible clock domains.

| Timestamp | Definition |
|---|---|
| `t_driver` | Optional driver timestamp with its source and clock flags recorded |
| `t_capture_return` | Host time immediately after a frame becomes available to the application |
| `t_infer_start` | Host time when the worker takes the frame |
| `t_pre_end` | Preprocessing complete |
| `t_gpu_done` | Result available to the consumer, including required synchronization |
| `t_detect_done` | Postprocessing complete |
| `t_control_release` | Scheduled absolute release of the control tick |
| `t_control_start` | Actual start of that tick |
| `t_command_ready` | Command produced by the controller |
| `t_bridge_receive` | Robot bridge receives the command |
| `t_write_done` | Robot command API returns; document what this completion means |
| `t_feedback` | Host receives the associated joint-state sample |

Required derived metrics:

- **Frame wait:** `t_infer_start - t_capture_return`.
- **Processing latency:** `t_detect_done - t_infer_start`.
- **Observation age at control:** `t_control_start - source_frame.t_capture_return`.
- **Application observation-to-command age:** `t_write_done - source_frame.t_capture_return`.
- **Control wake-up lateness:** `max(0, t_control_start - t_control_release)`.
- **Deadline miss:** the tick's required work finishes after its next scheduled release.
- **Bridge latency:** `t_write_done - t_command_ready`.
- **Feedback latency:** host-observed time to a qualifying position change, reported separately.

Report p50, p95, p99, maximum, sample count, and run duration. Also report CPU/GPU utilization, temperature, clock state, memory, and tracking quality.

Timestamping after `VideoCapture::read()` excludes earlier exposure, USB transfer, decode, and hidden driver buffering. Label the metric accordingly. V4L2 timestamps are useful only after confirming their clock and capture-source semantics [3].

Use CUDA events for device work and host timestamps for host-visible completion. Timing only an asynchronous enqueue understates inference latency. Do not add unnecessary synchronization to the optimized loop merely to simplify logging.

### Rates and provisional targets

A 60 FPS camera produces a nominal frame every 16.7 ms. Inference and control have separate clocks and workloads. An inference taking longer than 16.7 ms can still support a useful lower-rate controller.

| Metric | Initial engineering target | Decision rule |
|---|---|---|
| Camera delivery | Nominal 60 FPS in one verified supported mode | Verify cadence over 10 minutes; investigate sustained shortfall rather than trusting the requested setting |
| Inference | Start at 20 fresh results/s for the neural detector | Freeze a feasible model/input size after baseline measurement |
| Control | Start at 20 Hz, a 50 ms period | Select rate from measured command and feedback costs; do not promise 60 Hz actuator updates |
| Control wake-up lateness | p99 at most 5 ms idle; 10 ms under the defined load | Provisional soft real-time targets; log all deadline misses |
| Observation-to-command age | p95 at most 100 ms; p99 at most 150 ms | Host/application metric unless driver timestamps are validated |
| Command freshness | Initial maximum accepted observation age: 200 ms | A conservative starting limit to validate during robot bring-up |
| Stability | 30-minute integrated run with bounded queues and no sustained memory growth | Separate startup/model loading from steady state |
| Tracking | Center within 15% of image diagonal within 2 seconds for at least 90% of prescribed slow target moves | Freeze target distance, speed, lighting, and joint-range conditions |

These are proposed targets, not hardware specifications or promised outcomes. Finalize feasible budgets in Week 2 and robot limits in Week 6. Record any revision and its measured reason before comparing optimization results. A well-explained limit is preferable to changing the benchmark until it passes.

For memory, record process RSS/PSS and Jetson system RAM indicators. CPU and GPU share system memory on this platform; do not blindly sum overlapping reports.

### Minimum event record

`run_id, config_hash, event, host_monotonic_ns, frame_id, observation_id, command_id, queue_depth, status`

Keep stage events in a normalized event log; derive per-frame tables during analysis. Log metadata includes git commit, model hash, input dimensions, backend, precision, camera mode, power mode, clocks, temperature, and logging configuration.

## 6. Twelve week execution plan

### Month 1 — Establish an honest baseline

#### Week 1 — Hardware and scope freeze

**Work**

- Build and run the existing capture project on the Jetson.
- Record the Jetson model, JetPack/L4T, CUDA, TensorRT, OpenCV, compiler, and USB topology.
- Enumerate webcam modes with `v4l2-ctl --list-formats-ext`; choose a mode supporting 60 FPS.
- Measure real delivered cadence. Check whether exposure or auto-exposure changes reduce frame rate.
- Calibrate the SO-101 using the pinned LeRobot instructions [1]. Validate low-speed moves without vision.
- Select the camera arrangement, target, active joints, and physically accessible power disconnect.

**Done when**

A 10-minute camera-only run and a separate supervised robot communication session succeed. Produce a configuration record and one short hardware video. Resolve hardware setup before model optimization.

**Budget:** 10–12 hours.

#### Week 2 — Profiling and synchronous baseline

**Work**

- Implement frame IDs, stage events, a run manifest, and bounded logging.
- Add headless capture-only and synchronous color-tracking modes.
- Measure frame intervals, processing cost, allocation behavior, RSS/PSS, and camera buffering symptoms.
- Use a visible step change, such as an LED or display transition, to estimate hidden delay; report timing uncertainty.
- Establish a fixed warm-up, run length, and statistics script.
- Compare profiling enabled/disabled to estimate instrumentation overhead.

**Done when**

One command produces logs and a baseline table. Document exactly where timing begins and ends. Freeze provisional rates and latency budgets from measured evidence.

**Budget:** 10–12 hours.

#### Week 3 — Neural inference baseline

**Work**

- Select one small pretrained detector with a straightforward ONNX export and a target class already supported.
- Use a working FP32 reference backend, preferably OpenCV DNN on CPU if supported without custom work.
- Fix resize/letterboxing, color order, normalization, confidence threshold, and NMS.
- Run a small saved-frame correctness set and a repeatable video workload.
- Keep the colored-marker mode for hardware troubleshooting.

**Done when**

The neural detector runs on Jetson with per-stage measurements and visual checks. No model training is needed. Tag the perception baseline.

**Budget:** 10–12 hours. If model compatibility consumes more than two sessions, switch to a simpler supported detector.

#### Week 4 — Buffering and concurrency experiment

**Work**

- Implement or validate the latest-frame mailbox and reusable frame pool.
- Compare synchronous processing, a small bounded FIFO, and latest-frame processing on the same workload.
- Inject fixed processing delays of 25, 50, and 100 ms.
- Count produced, processed, replaced, and rejected frames separately.
- Verify ownership, shutdown, empty-mailbox waits, and inference-worker failure behavior.

**Done when**

A comparison shows how queue policy affects age, throughput, and loss. Queues and memory remain bounded. Demonstrate recovery to fresh frames after overload ends.

**Budget:** 10–12 hours.

**Month 1 gate:** Capture and inference work on Jetson; timestamp limitations are documented; baseline and mailbox results are reproducible. If behind, drop the second active joint and optional camera-backend work.

### Month 2 — Close the physical loop and optimize

#### Week 5 — Robot interface and watchdog

**Work**

- Create the C++ command/feedback contract and a small LeRobot bridge.
- Benchmark position writes and joint-state reads independently of vision.
- Add command sequence numbers, bounded message handling, and expiry checks inside the bridge.
- Establish explicit states: disconnected, ready, tracking, target-lost, fault, and stopped.
- Test stale commands and worker failure with a mock robot before supervised hardware tests.

**Done when**

A scripted sequence produces command and feedback timing logs. Expired or out-of-order commands never reach the robot interface. The bridge handles loss of the main runtime.

**Budget:** 10–12 hours.

#### Week 6 — First integrated tracking demo

**Work**

- Connect detections to a fixed-rate controller.
- Calibrate one joint's image response, then the second joint only if stable.
- Add deadband, position/rate limits, saturation reporting, and target-loss handling.
- Trace camera-to-command IDs across all processes.
- Measure physical tracking separately from software timing.

**Done when**

The arm follows slow target motion within its restricted workspace. Run 10 supervised minutes with matching logs and video. Freeze camera geometry, control rate, and test trajectories.

**Budget:** 10–12 hours.

**Integration gate:** Demonstrate correct bounded behavior and explain the latency distribution before doing further performance work.

#### Week 7 — TensorRT inference optimization

**Work**

- Build a TensorRT engine on a compatible target stack; record build settings and versions.
- Establish TensorRT FP32 parity where supported, then compare FP16 using the same model and inputs.
- Separate preprocessing, transfers, enqueue, device execution, synchronization, and postprocessing.
- Warm up before steady-state runs. Use batch size one.
- Check detector outputs and physical tracking before declaring a speedup.

**Done when**

Publish both isolated inference and integrated pipeline comparisons. An optimization is accepted only with documented output-quality and tracking effects.

**Budget:** 10–12 hours.

#### Week 8 — Optimize the measured bottleneck

**Work**

Choose at most two changes supported by Week 7 profiles:

- Reduce repeated frame/tensor allocation and unnecessary clones.
- Reuse host/device buffers; evaluate pinned staging only if transfers are significant.
- Avoid repeated resize/color conversions.
- Change OpenCV/GStreamer/V4L2 capture buffering if hidden camera latency dominates.
- Move blocking feedback reads away from the control release path.
- Reduce lock hold time or logging interference.

If implementing V4L2 MMAP, track dequeue/requeue ownership and timestamps explicitly. Do not call that whole-pipeline zero-copy: decode, conversion, tensor preparation, and GPU transfers may still copy data.

**Done when**

Each accepted change has a one-variable A/B result and a documented tradeoff. Keep ineffective changes out of the final default path.

**Budget:** 10–12 hours.

**Month 2 gate:** A neural detector drives the arm; the optimized pipeline has traceable measurements and at least one substantiated performance improvement or a documented hardware-bound limit.

### Month 3 — Characterize reliability and produce evidence

#### Week 9 — Scheduling and thermal characterization

**Work**

- Run idle, CPU-load, and controlled I/O-load conditions.
- Measure control wake-up lateness, deadlines, observation age, and bridge behavior.
- Compare the normal Jetson power configuration with one explicitly recorded alternative where supported.
- Record `tegrastats`, clocks, temperatures, and power mode [2].
- Try CPU affinity only if scheduling contention is measured; record the tradeoff.
- Prefer absolute periodic releases and small critical sections before considering scheduler-policy changes.

**Done when**

Produce latency distributions by load and power condition. Separate scheduling effects from model execution and thermal effects.

**Budget:** 10–12 hours.

#### Week 10 — Fault injection and long runs

**Work**

- Test camera loss, target disappearance, inference stall, bridge loss, serial error, and logging/storage pressure.
- Begin with mocks or robot disconnected; repeat appropriate cases under supervision.
- Run the integrated system for 30 minutes; run a 60-minute perception-only soak.
- Check memory growth, frame-pool exhaustion, stale observations, and reconnect behavior.
- Require explicit re-arming after faults.

**Done when**

The fault matrix records expected action, observed detection time, command suppression, and recovery. No test leaves an expired motion sequence queued for later execution.

**Budget:** 10–12 hours.

#### Week 11 — Freeze and reproduce results

**Work**

- Freeze a baseline commit and optimized commit.
- Run the final benchmark matrix using identical workloads and conditions.
- Repeat each primary configuration three times.
- Complete 50 prescribed slow target transitions, using five positions and ten repetitions, or an equivalently documented repeatable sequence.
- Count target loss, non-detection, joint saturation, and timeouts as outcomes; do not omit failed transitions.
- Rerun setup from the README or have another person follow it.

**Done when**

The results include raw logs, scripts, distributions, accuracy checks, tracking success, and negative findings. A clean build and repeat run succeed.

**Budget:** 10–12 hours.

#### Week 12 — Package the project

**Work**

- Write the architecture and benchmark reports.
- Record a two-to-three-minute demonstration with a matching run ID.
- Explain one difficult bug, one optimization that worked, and one approach that failed.
- Add known limitations: timestamp uncertainty, bridge overhead, simple control, and hardware bounds.
- Prepare an interview walkthrough and factual resume wording using measured results.

**Done when**

A reviewer can build, run, inspect, and reproduce the main result. Tag a release. Publish externally only after reviewing model/code licenses and removing private information.

**Budget:** 8–10 hours plus remaining recovery time.

## 7. Required experiment matrix

Run primary software comparisons on the same recorded workload first, then validate behavior live. Use a paced replay source for queue experiments: preserve original frame timing, with a separate documented unpaced throughput mode if needed.

| Experiment | Controlled change | Main question |
|---|---|---|
| B0 | Capture only | Is 60 FPS actually delivered, and with what cadence? |
| B1 | Synchronous FP32 reference detector | What is the original processing cost? |
| B2 | Bounded FIFO under injected delays | How much stale work accumulates? |
| B3 | Latest-frame mailbox under identical delays | How does dropping affect freshness and useful throughput? |
| O1 | TensorRT FP32, when available | What changes from the inference runtime alone? |
| O2 | TensorRT FP16 | What changes from precision, including detection quality? |
| O3 | One buffer/copy optimization | Does measured data movement improve? |
| R1 | Robot enabled versus disabled | What does command/feedback integration add? |
| R2 | Idle versus defined system load | How do timing tails and deadlines change? |
| R3 | Startup versus thermal steady state | Does a short benchmark misrepresent sustained behavior? |
| F1 | Fault injection | Are stale data and lost workers handled as designed? |

For primary performance runs: approximately 30 seconds warm-up followed by 10 minutes of measurement, three repetitions. Keep identical input resolution, model, thresholds, and control settings unless that variable is explicitly being tested. Alternate baseline/optimized run order when practical.

Add a short command-expiry/freshness check and saved-frame output comparison after each meaningful runtime change. Exercise mailbox ownership and shutdown with a focused concurrency test; use sanitizers on a supported host configuration where practical. Avoid building a large generic test framework.

## 8. Failure behavior

The robot adapter must not keep accepting increasingly old commands simply because the controller is alive.

| Fault | Required behavior |
|---|---|
| Target unavailable or observation too old | Stop issuing new tracking increments; enter target-lost state |
| Inference worker stalls | Control remains responsive; age limit blocks stale detections |
| Main C++ runtime exits | Bridge watchdog detects command expiry and enters its validated stop behavior |
| Camera disconnects | Mark capture unavailable and invalidate tracking observations |
| Bridge/serial connection fails | Invalidate the robot state; do not accumulate a replay queue |
| Frame pool exhausted | Apply a documented nonblocking drop policy; increment a counter |
| Logging falls behind | Drop bounded diagnostic records and count loss; never block control indefinitely |
| Reconnection | Validate state and require explicit re-arming before motion |

Determine the stop action on the actual arm. Holding the last position may retain torque; disabling torque may allow gravity-driven motion. Neither is automatically safe. Establish a supported pose, limits, and a reachable physical disconnect before live experiments. A software watchdog cannot stop motion if the bridge or hardware itself is unresponsive.

Watchdog response is sampled by the bridge, so detection can occur after the nominal expiry threshold by its check interval and scheduling delay. Measure this response instead of claiming an exact upper bound from a configured timeout.

## 9. Repository deliverables

The finished repository should contain:

- `README.md`: hardware, pinned dependencies, build, calibration reference, and run instructions.
- `docs/architecture.md`: threads/processes, ownership, timestamps, and fault states.
- `docs/measurement_contract.md`: metric definitions, clock domains, and sensor/actuation limitations.
- `docs/benchmark_report.md`: matched comparisons, plots, raw run IDs, and limitations.
- `configs/`: camera, model, control, limits, and benchmark configurations.
- `scripts/`: build/run helpers, replay, load injection, and analysis.
- `tests/`: focused ownership, expiry, shutdown, and detection-comparison checks.
- A small redistributable evaluation workload or instructions to recreate it.
- A manifest for larger raw logs and model files rather than committing every artifact.
- A demo video or link tied to one recorded run.
- Baseline and final release tags.

Every claimed improvement must identify the baseline, measurement endpoints, workload, hardware configuration, sample count, and output-quality check. Report relative improvement and absolute values.

For example, write a resume bullet only after the measurements exist:

> Built a C++ camera-to-actuator runtime on Jetson Orin Nano and SO-101; reduced [defined metric] p99 from [measured baseline] to [measured result] through [validated changes], while maintaining [measured tracking criterion].

Brackets are placeholders, not anticipated results.

## 10. Weekly working rhythm and scope control

Suggested weekly allocation:

- Two 90-minute sessions for implementation or reading.
- One two-hour session for profiling and analysis.
- One four-to-five-hour weekend session for hardware integration and repeatable runs.
- Thirty minutes to record findings and choose the next experiment.

At the end of every week, record: what changed, what was measured, what remains unexplained, and the next falsifiable hypothesis.

If the plan slips, cut scope in this order:

1. Optional power-mode comparisons and direct V4L2 work.
2. The second active joint.
3. Pinned-buffer or GPU preprocessing experiments.
4. Extra visualization and UI work.

Preserve the measurement contract, baseline, bounded queues, robot integration, fault tests, and final report.

### Optional follow-on work

After the required release, choose one extension according to the remaining gap:

- **Networking:** split observation/command transport across machines and measure delay, loss, and clock alignment.
- **Camera systems:** add a second camera and study synchronization and buffering.
- **Native actuator integration:** replace the Python bridge with C++ and compare measured overhead.
- **Learned-policy runtime:** replace the simple controller with a policy while retaining the instrumentation and watchdogs.

These extensions are outside the 12-week commitment.

## 11. Final acceptance checklist

- [ ] Webcam mode and sustained cadence are verified, not inferred from an API request.
- [ ] Capture, inference, and control rates are reported separately.
- [ ] CPU and GPU timing boundaries are explicit.
- [ ] Every command links to its source observation/frame.
- [ ] Latest-frame behavior and bounded memory are demonstrated under overload.
- [ ] Robot command and feedback costs are measured.
- [ ] Expired commands are rejected by the robot bridge itself.
- [ ] Tracking quality and failures are reported alongside performance.
- [ ] Final comparisons use matched workloads and repeated runs.
- [ ] Thermal, clock, power, and software configurations are recorded.
- [ ] Timing uncertainty and unmeasured sensor/physical-actuation latency are disclosed.
- [ ] One engineer can reproduce the main result from the README.
- [ ] Final claims contain measured values rather than provisional targets.

## 12. Technical references

Documentation checked October 23, 2026. Pin software versions and use the matching documentation rather than assuming examples from the newest release work on the installed JetPack.

1. [Hugging Face LeRobot SO-101 guide](https://huggingface.co/docs/lerobot/main/en/so101) — robot setup, connection, and calibration.
2. [NVIDIA Jetson Linux R36.4.4 power and performance guide](https://docs.nvidia.com/jetson/archives/r36.4.4/DeveloperGuide/SD/PlatformPowerAndPerformance/JetsonOrinNanoSeriesJetsonOrinNxSeriesAndJetsonAgxOrinSeries.html) — Orin power modes, clocks, and monitoring. Use the installed release's equivalent.
3. [Linux V4L2 buffer documentation](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/buffer.html) — buffer ownership, timestamps, and timestamp-source flags.
4. [NVIDIA TensorRT performance guidance](https://docs.nvidia.com/deeplearning/tensorrt/latest/performance/best-practices.html) — entry point for inference benchmarking and optimization guidance; consult the installed TensorRT version.

The schedule, budgets, target rates, acceptance thresholds, and architecture choices in this plan are engineering proposals for this project, not performance guarantees from these references.

