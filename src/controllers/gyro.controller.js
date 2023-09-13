const gyro = require('../models/gyro');
const gyroCtrl = {};

readsCtrl.saveData = async(req, res) => {
    const lectura = new gyro(req.body);
    await lectura.save();

    res.json({
        status: 201,
        msg: "gyro saved"
    });
}

module.exports = gyroCtrl; 