## optimization 1 design
### fixed pool size frame buffer 

               ┌─────────────┐
               │ free_pool_  │
               │ A B C D     │
               └──────┬──────┘
                      │ acquire
                      ▼
CameraCapture ─────► buffer A
                      │
                      ▼
               ┌─────────────┐
               │latest_slot_ │
               │      A      │
               └──────┬──────┘
                      │ pop
                      ▼
                 Consumer A
                      │
                      │ release
                      ▼
               ┌─────────────┐
               │ free_pool_  │
               │ B C D A     │
               └─────────────┘


