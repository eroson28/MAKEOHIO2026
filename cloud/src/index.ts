import express, { Application, Request, Response } from "express";
import fs from "fs";

const app: Application = express();
const port = 3911; // The port your express server will be running on.

let curTitle: string = "";
let curCaption: string = "";
let curColorR: number = 0;
let curColorG: number = 0;
let curColorB: number = 0;
let curCamera: string = "";

function hexToRgb(hex: string): number[] {
    const r = parseInt(hex.slice(1, 3), 16);
    const g = parseInt(hex.slice(3, 5), 16);
    const b = parseInt(hex.slice(5, 7), 16);
    return [ r, g, b ];
}

// Enable URL-encoded form data parsing
app.use(express.urlencoded({ extended: true }));

// Middleware to parse JSON bodies
app.use(express.json());

app.use(express.raw({ type: "application/octet-stream", limit: "10mb" }));

/**
 * The arduino requests what the current status is, whenever it deems it fit.
 */
app.get("/status", (req: Request, res: Response) => {
    res.json({
        success: 1,
        caption: curCaption,
        title: curTitle,
        r: curColorR,
        g: curColorG,
        b: curColorB,
    })
});

/**
 * The webapp requests a snapshot of what the arduino sees.
 */
app.get("/camera-latest", (req: Request, res: Response) => {
    const img = fs.readFileSync("image.png");
    res.set("Content-Type", "image/png");
    res.send(img);    
});

/**
 * The webapp can set the status.
 */
app.post("/set-status", (req: Request, res: Response) => {
    console.log(req.body);
    const { caption, title, color } = req.body;
    const [r, g, b] = hexToRgb(color);
    console.log(`Webapp wants to set status to ${caption} ${title} ${[r, g, b]}`);
    curCaption = caption;
    curTitle = title;
    curColorR = r;
    curColorR = g;
    curColorR = b;
    res.send({ success: true });
});

/**
 * The arduino sends what it is seeing.
 */
app.post("/set-camera", (req: Request, res: Response) => {
    const { camera } = req.body;
    curCamera = camera;
    fs.writeFileSync("public/camera.png", req.body);
    res.send({ success: true });
});

app.use(express.static('frontend'));


// Start the server
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});