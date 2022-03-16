const express = require('express')
const app = express()
const port = 8081

app.set('etag', false)

app.get('/with_module', (req, res) => {

    res.status(200).send(
        `   <p>Visitor id : ${req.get('x-visitor-id')}</p>
            <p>Visitor context : ${req.get('x-visitor-context')}</p>
            <p>Flags : ${req.get('x-flagship-flags')}</p>
        `)
})

app.get('/without_module', (req, res) => {
    if (req.get('user-agent').includes("Chrome")) {
        res.send("Chrome")
    } else {
        res.send("Firefox")
    }
})

app.listen(port, () => {
    console.log(`Listening on port ${port}`)
})