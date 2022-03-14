const express = require('express')
const app = express()
const port = 8081

app.get('/test', (req, res) => {
    console.log("flags " + req.get('x-flagship-flags'))
    console.log("user agent " + req.get('user-agent'))
    if (req.get('user-agent').includes("Chrome")) {
        res.send("Chrome")
    } else {
        res.send("Firefox")
    }
})

app.get('/experiment', (req, res) => {
    console.log("flags " + req.get('x-flagship-flags'))
    console.log("user agent " + req.get('user-agent'))
    if (req.get('user-agent').includes("Chrome")) {
        res.send("Chrome")
    } else {
        res.send("Firefox")
    }
})

app.listen(port, () => {
    console.log(`Example app listening on port ${port}`)
})