const express = require('express')
const app = express()
const port = 8081
const { Flagship } = require("@flagship.io/js-sdk");

app.set('etag', false)

app.get('/with_module', (req, res) => {

    res.status(200).send(
        `   <p>Visitor id : ${req.get('x-visitor-id')}</p>
            <p>Visitor context : ${req.get('x-visitor-context')}</p>
            <p>Flags : ${req.get('x-flagship-flags')}</p>
        `)
})

app.get('/without_module', (req, res) => {

    Flagship.start("c0n48jn5thv01k0ijmo0", "BsIK86oh7c12c9G7ce4Wm1yBlWeaMf3t1S0xyYzI");

    const visitor_id = req.get('x-visitor-id')
    const visitor_context = req.get('x-visitor-context')
    const [visitorContextKey, visitorContextValue] = visitor_context.split(':');

    const visitor = Flagship.newVisitor({
        visitorId: visitor_id,
        context: { [visitorContextKey]: visitorContextValue }
    });

    visitor.on("ready", (error) => {
        if (error) {
            res.status(500).send("Internal Error (Visitor)")
            return;
        }
        const flagKey = visitor.getFlag("IsVIP", false)._key;
        const flagValue = visitor.getFlag("IsVIP", false).getValue();
        res.status(200).send(
            `   <p>Visitor id : ${visitor_id}</p>
                <p>Visitor context : ${visitor_context}</p>
                <p>Flags : ${flagKey}: ${flagValue}</p>
            `)
    });
})

app.listen(port, () => {
    console.log(`Listening on port ${port}`)
})