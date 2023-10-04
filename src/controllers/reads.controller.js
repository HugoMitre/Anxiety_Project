const reads = require('../models/reads');
const readsCtrl = {};

// readsCtrl.getData = async(req, res) => {
//     const data = await reads.find();
//     // await reads.collection.drop();
//     res.json(data);
// }

readsCtrl.saveData = async(req, res) => {
    console.log(req);
    const lectura = new reads(req.body);
    await lectura.save();

    res.json({
        status: 201,
        msg: "read saved"
    });
}

module.exports = readsCtrl; 