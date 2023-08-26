const reads = require('../models/reads');
const phase = require('../models/phase');
const ExcelJS = require('exceljs');
const phaseCtrl = {};

phaseCtrl.saveSensorData = async(req, res) => {
    const data = await reads.find({}).toArray();
    await reads.collection.drop();

    console.log(req.body);

    const fase = new phase();
    fase.subject = req.body.subject;
    fase.phase = req.body.phase;
    fase.data_reads = data;
    fase.date = new Date();

    await fase.save();

    // Export to excel
    const workbook = new ExcelJS.Workbook();
    const worksheet = workbook.addWorksheet('Datos');

    // Define las columnas en el archivo Excel
    worksheet.columns = [
        { header: 'Time', key: 'time', width: 15 },
        { header: 'IR', key: 'ir', width: 10 },
        { header: 'Red', key: 'red', width: 10 },
        { header: 'HR', key: 'hr', width: 10 },
        { header: 'SpO2', key: 'spo2', width: 10 },
        { header: 'Gyro', key: 'gyro', width: 10 },
    ];

    // Agrega los datos a la hoja de cálculo
    data.forEach((row) => {
        worksheet.addRow({
        time: row.time,
        ir: row.ir,
        red: row.red,
        hr: row.hr,
        spo2: row.spo2,
        gyro: row.gyro
        });
    });

    // Genera un nombre de archivo único 
    const fileName = `${req.body.subject}_${req.body.phase}_${Date.now()}.xlsx`;

    // Guarda el archivo en el servidor y envía el enlace de descarga al cliente
    await workbook.xlsx.writeFile(fileName);

    res.redirect('/api/phase');
}

module.exports = phaseCtrl;