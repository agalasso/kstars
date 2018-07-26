/*  Ekos PHD2 Handler
    Copyright (C) 2016 Jasem Mutlaq <mutlaqja@ikarustech.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#pragma once

#include "../guideinterface.h"
#include "fitsviewer/fitsview.h"
#include <QPointer>

#include <QAbstractSocket>
#include <QJsonArray>
#include <QTimer>

class QTcpSocket;

namespace Ekos
{
/**
 * @class  PHD2
 * Uses external PHD2 for guiding.
 *
 * @author Jasem Mutlaq
 * @version 1.1
 */
class PHD2 : public GuideInterface
{
    Q_OBJECT

  public:
    typedef enum {
        Version,
        LockPositionSet,
        CalibrationComplete,
        StarSelected,
        StartGuiding,
        Paused,
        StartCalibration,
        AppState,
        CalibrationFailed,
        CalibrationDataFlipped,
        LoopingExposures,
        LoopingExposuresStopped,
        SettleBegin,
        Settling,
        SettleDone,
        StarLost,
        GuidingStopped,
        Resumed,
        GuideStep,
        GuidingDithered,
        LockPositionLost,
        Alert,
        GuideParamChange
    } PHD2Event;
    typedef enum {
        STOPPED,
        SELECTED,
        LOSTLOCK,
        PAUSED,
        LOOPING,
        CALIBRATING,
        CALIBRATION_FAILED,
        CALIBRATION_SUCCESSFUL,
        GUIDING,
        DITHERING,
        DITHER_FAILED,
        DITHER_SUCCESSFUL
    } PHD2State;
    typedef enum {
        DISCONNECTED,
        CONNECTED,
        EQUIPMENT_DISCONNECTED,
        EQUIPMENT_CONNECTED
    } PHD2Connection;
    typedef enum {
        PHD2_UNKNOWN,
        PHD2_RESULT,
        PHD2_EVENT,
        PHD2_ERROR,
    } PHD2MessageType;

    //These are the PHD2 Results and the commands they are associated with
    typedef enum {
        NO_RESULT,
            //capture_single_frame
        CLEAR_CALIBRATION_COMMAND_RECEIVED,     //clear_calibration
        DITHER_COMMAND_RECEIVED,                //dither
            //find_star
            //flip_calibration
            //get_algo_param_names
            //get_algo_param
            //get_app_state
            //get_calibrated
            //get_calibration_data
        IS_EQUIPMENT_CONNECTED,                 //get_connected
            //get_cooler_status
            //get_current_equipment
        DEC_GUIDE_MODE,                         //get_dec_guide_mode
        EXPOSURE_TIME,                          //get_exposure
        EXPOSURE_DURATIONS,                     //get_exposure_durations
            //get_lock_position
            //get_lock_shift_enabled
            //get_lock_shift_params
            //get_paused
        PIXEL_SCALE,                            //get_pixel_scale
            //get_profile
            //get_profiles
            //get_search_region
            //get_sensor_temperature
        STAR_IMAGE,                             //get_star_image
            //get_use_subframes
        GUIDE_COMMAND_RECEIVED,                 //guide
            //guide_pulse
            //loop
            //save_image
            //set_algo_param
        CONNECTION_RESULT,                      //set_connected
        SET_DEC_GUIDE_MODE_COMMAND_RECEIVED,    //set_dec_guide_mode
        SET_EXPOSURE_COMMAND_RECEIVED,          //set_exposure
            //set_lock_position
            //set_lock_shift_enabled
            //set_lock_shift_params
        SET_PAUSED_COMMAND_RECEIVED,            //set_paused
            //set_profile
            //shutdown
        STOP_CAPTURE_COMMAND_RECEIVED           //stop_capture
    } PHD2ResultType;

    PHD2();
    ~PHD2();

    //These are the connection methods to connect the external guide program PHD2
    bool Connect() override;
    bool Disconnect() override;
    bool isConnected() override { return (connection == CONNECTED || connection == EQUIPMENT_CONNECTED); }

    //These are the PHD2 Methods.  Only some are implemented in Ekos.

        //capture_single_frame
    bool clearCalibration() override;       //clear_calibration
    bool dither(double pixels) override;    //dither
        //find_star
        //flip_calibration
        //get_algo_param_names
        //get_algo_param
        //get_app_state
        //get_calibrated
        //get_calibration_data
    void checkIfEquipmentConnected();       //get_connected
        //get_cooler_status
        //get_current_equipment
    void checkDEGuideMode();                //get_dec_guide_mode
    void requestExposureTime();             //get_exposure
    void requestExposureDurations();        //get_exposure_durations
        //get_lock_position
        //get_lock_shift_enabled
        //get_lock_shift_params
        //get_paused
    void requestPixelScale();               //get_pixel_scale
        //get_profile
        //get_profiles
        //get_search_region
        //get_sensor_temperature
    void requestStarImage(int size);        //get_star_image
        //get_use_subframes
    bool guide() override;                  //guide
        //guide_pulse
        //loop
        //save_image
        //set_algo_param
    void connectEquipment(bool enable);//set_connected
    void requestSetDEGuideMode(bool deEnabled, bool nEnabled, bool sEnabled);           //set_dec_guide_mode
    void requestSetExposureTime(int time);  //set_exposure
        //set_lock_position
        //set_lock_shift_enabled
        //set_lock_shift_params
    bool suspend() override;                //set_paused
    bool resume() override;                 //set_paused
        //set_profile
        //shutdown
    bool abort() override;                  //stop_capture

    bool calibrate() override; //Note PHD2 does not have a separate calibrate command.  This is unused.
    void setGuideView(FITSView *guideView);

  private slots:

    void readPHD2();
    void displayError(QAbstractSocket::SocketError socketError);

  private:
    QPointer<FITSView> guideFrame;

    QVector<QPointF> errorLog;

    void sendPHD2Request(const QString &method, const QJsonArray &args = QJsonArray());
    void sendJSONRPCRequest(const QString &method, const QJsonArray &args = QJsonArray());

    void processPHD2Event(const QJsonObject &jsonEvent);
    void processPHD2Result(const QJsonObject &jsonObj, const QByteArray &rawResult);
    void processStarImage(const QJsonObject &jsonStarFrame);
    void processPHD2State(const QString &phd2State);
    void processPHD2Error(const QJsonObject &jsonError);

    PHD2ResultType takeRequestFromList(const QJsonObject &response);

    QTcpSocket *tcpSocket { nullptr };
    int nextRpcId { 1 };

    QHash<QString, PHD2Event> events;                     // maps event name to event type
    QHash<QString, PHD2ResultType> methodResults;         // maps method name to result type
    QVector<QPair<int, PHD2ResultType>> resultRequests;   // result type for each RPC id

    PHD2State state { STOPPED };
    PHD2Connection connection { DISCONNECTED };
    PHD2Event event { Alert };
    uint8_t setConnectedRetries { 0 };

    void setEquipmentConnected();
    void updateGuideParameters();

    QTimer *abortTimer;
    QTimer *ditherTimer;
    int starReAcquisitionTime=5000;

    double pixelScale=0;

    QString logValidExposureTimes;
};
}
