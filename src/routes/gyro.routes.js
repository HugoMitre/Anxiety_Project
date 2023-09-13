// Rutas que se crearán para los endpoints que correspondan al envío de datos del arduino a local.
const express = require('express');
const router = express.Router();
const gyroCtrl = require('../controllers/gyro.controller');

router.post('/', gyroCtrl.saveData);

module.exports = router;