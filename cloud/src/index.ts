import express, { Application, Request, Response } from "express";
import fs from "fs";

const app: Application = express();
const port = 3000; // The port your express server will be running on.

let curStatus = {
    caption: "Hello",
    title: "World",
    color: "255 0 0",
};
let curCamera: string = "";

// Enable URL-encoded form data parsing
app.use(express.urlencoded({ extended: true }));

// Middleware to parse JSON bodies
app.use(express.json());

app.use(express.raw({ type: "application/octet-stream", limit: "10mb" }));
app.use(express.static('frontend'));

/**
 * The arduino requests what the current status is, whenever it deems it fit.
 */
app.get("status", (req: Request, res: Response) => {
    res.json({
        success: 1,
        caption: "Internet and welcome to Game Theory",
        title: "Hello",
        r: 0,
        g: 255,
        b: 0
    })
});

/**
 * The webapp requests a snapshot of what the arduino sees.
 */
app.get("camera-latest", (req: Request, res: Response) => {
    const img = fs.readFileSync("image.png");
    res.set("Content-Type", "image/png");
    res.send(img);    
});

/**
 * The webapp can set the status.
 */
app.get("set-status", (req: Request, res: Response) => {
    const { caption, title, r, g, b } = req.body;
    console.log(`Webapp wants to set status to ${caption} ${title} ${r} ${g} ${b}`);
});

/**
 * The arduino sends what it is seeing.
 */
app.post("set-camera", (req: Request, res: Response) => {
    const { camera } = req.body;
    curCamera = camera;
    fs.writeFileSync("public/camera.png", req.body);
    res.send({ success: true });
});

// Start the server
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});