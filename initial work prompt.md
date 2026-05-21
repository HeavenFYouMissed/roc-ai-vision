initial work prompt
 Initialize our clean workspace based on the rules specified in our root `cursor.md` file. 

As a Lead Robotics Research Scientist, do not output code during this initial phase. I want you to perform the following initialization tasks autonomously:

1. Conduct a professional web research review on optimal modular directory structures for a real-time ROS 2 computer vision and state-estimation tracking pipeline.
2. Create a folder named `workspace_blueprint/` in our project root.
3. Inside `workspace_blueprint/`, generate a `project_tree.md` file mapping out our custom multi-package layout:
   - `vision_pipeline/` (OpenCV frame acquisition & generic ONNX Runtime inference engine)
   - `kinematics_engine/` (Constant Acceleration state matrices and predictive tracking logic)
   - `tracker_interfaces/` (Custom ROS 2 .msg specifications linking the vision and math layers)
   - `frontend_visualizer/` (SvelteKit real-time SVG telemetry canvas)
4. Generate a `research_notes.md` file inside `workspace_blueprint/` detailing standard C++17 implementations for dynamic ONNX model metadata querying (`Ort::Session`), dynamic letterboxing matrix scaling, and the matrix dimension profiles required for human-surveillance feature tracking.

Once you have established this architectural blueprint and verified your workspace notes, provide a concise summary of the layout so we can spawn parallel worker agents for the individual node modules.