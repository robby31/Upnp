#include "servicecontentdirectory.h"

ServiceContentDirectory::ServiceContentDirectory(MediaRendererModel *model, UpnpDevice *upnpParent, QObject *parent):
    AbstractService(upnpParent, parent),
    m_renderersModel(model)
{
    initDescription();
    initStateVariables();
    initActions();
    emit descriptionChanged();

    if (upnpParent)
        rootFolder.setHostUrl(upnpParent->url());

    rootFolder.setNetworkAccessManager(networkManager());

    connect(&rootFolder, SIGNAL(scanFolder(QString)), this, SIGNAL(scanFolder(QString)));

    connect(this, SIGNAL(folderAdded(QString)), this, SLOT(folderAddedSlot(QString)));

    connect(this, SIGNAL(addFolderSignal(QString)), &rootFolder, SLOT(addFolderSlot(QString)));
    connect(&rootFolder, SIGNAL(folderAddedSignal(QString)), this, SIGNAL(folderAdded(QString)));
    connect(&rootFolder, SIGNAL(error_addFolder(QString)), this, SIGNAL(error_addFolder(QString)));

    connect(this, SIGNAL(addNetworkLinkSignal(QString)), &rootFolder, SLOT(addNetworkLink(QString)));
    connect(&rootFolder, SIGNAL(linkAdded(QString)), this, SIGNAL(linkAdded(QString)));
    connect(&rootFolder, SIGNAL(error_addNetworkLink(QString)), this, SIGNAL(error_addNetworkLink(QString)));

    connect(this, SIGNAL(reloadLibrarySignal(QStringList)), &rootFolder, SLOT(reloadLibrary(QStringList)));

    connect(this, SIGNAL(incrementCounterPlayedSignal(QString)), &rootFolder, SLOT(incrementCounterPlayed(QString)));
    connect(this, SIGNAL(updateMediaData(QString,QHash<QString,QVariant>)), &rootFolder, SLOT(updateLibrary(QString,QHash<QString,QVariant>)));
    connect(this, SIGNAL(updateMediaFromId(int,QHash<QString,QVariant>)), &rootFolder, SLOT(updateLibraryFromId(int,QHash<QString,QVariant>)));

    m_streamingThread = new QThread();
    m_streamingThread->setObjectName("Streaming thread");
    connect(this, SIGNAL(destroyed(QObject*)), m_streamingThread, SLOT(quit()));
    connect(m_streamingThread, SIGNAL(finished()), m_streamingThread, SLOT(deleteLater()));
    m_streamingThread->start();
}

ServiceContentDirectory::~ServiceContentDirectory()
{
    if (m_streamingThread)
    {
        m_streamingThread->quit();
        if (!m_streamingThread->wait())
            qWarning() << "streaming Thread not finished.";
    }
}

void ServiceContentDirectory::initDescription()
{
    auto serviceDescription = new UpnpServiceDescription();
    serviceDescription->setServiceAttribute("serviceType", "urn:schemas-upnp-org:service:ContentDirectory:1");
    serviceDescription->setServiceAttribute("serviceId", "urn:upnp-org:serviceId:ContentDirectory");
    serviceDescription->setServiceAttribute("SCPDURL", "/UPnP_AV_ContentDirectory_1.0.xml");
    serviceDescription->setServiceAttribute("controlURL", "/upnp/control/content_directory");
    serviceDescription->setServiceAttribute("eventSubURL", "/upnp/event/content_directory");

    setInfo(serviceDescription->xmlInfo());

    setDescription(serviceDescription);
}

void ServiceContentDirectory::initActions()
{
    UpnpServiceDescription *serviceDescription = description();

    if (description())
    {
        QDomElement action = serviceDescription->addAction("GetSearchCapabilities");
        serviceDescription->addArgument(action, "SearchCaps", UpnpServiceDescription::OUT, "SearchCapabilities");

        action = serviceDescription->addAction("GetSortCapabilities");
        serviceDescription->addArgument(action, "SortCaps", UpnpServiceDescription::OUT, "SortCapabilities");

        action = serviceDescription->addAction("GetSystemUpdateID");
        serviceDescription->addArgument(action, "Id", UpnpServiceDescription::OUT, "SystemUpdateID");

        action = serviceDescription->addAction("Browse");
        serviceDescription->addArgument(action, "ObjectID", UpnpServiceDescription::IN, "A_ARG_TYPE_ObjectID");
        serviceDescription->addArgument(action, "BrowseFlag", UpnpServiceDescription::IN, "A_ARG_TYPE_BrowseFlag");
        serviceDescription->addArgument(action, "Filter", UpnpServiceDescription::IN, "A_ARG_TYPE_Filter");
        serviceDescription->addArgument(action, "StartingIndex", UpnpServiceDescription::IN, "A_ARG_TYPE_Index");
        serviceDescription->addArgument(action, "RequestedCount", UpnpServiceDescription::IN, "A_ARG_TYPE_Count");
        serviceDescription->addArgument(action, "SortCriteria", UpnpServiceDescription::IN, "A_ARG_TYPE_SortCriteria");
        serviceDescription->addArgument(action, "Result", UpnpServiceDescription::OUT, "A_ARG_TYPE_Result");
        serviceDescription->addArgument(action, "NumberReturned", UpnpServiceDescription::OUT, "A_ARG_TYPE_Count");
        serviceDescription->addArgument(action, "TotalMatches", UpnpServiceDescription::OUT, "A_ARG_TYPE_Count");
        serviceDescription->addArgument(action, "UpdateID", UpnpServiceDescription::OUT, "A_ARG_TYPE_UpdateID");
    }
    else
    {
        qCritical() << "invalid service description";
    }
}

void ServiceContentDirectory::initStateVariables()
{
    UpnpServiceDescription *serviceDescription = description();

    if (description())
    {
        QDomElement stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_ObjectID", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Result", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_BrowseFlag", false, false, "string");
        QStringList l_values;
        l_values << "BrowseMetadata" << "BrowseDirectChildren";
        serviceDescription->addAllowedValueList(stateVariable, l_values);

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Filter", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_SortCriteria", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Index", false, false, "ui4");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_Count", false, false, "ui4");

        stateVariable = serviceDescription->addStateVariable("A_ARG_TYPE_UpdateID", false, false, "ui4");

        stateVariable = serviceDescription->addStateVariable("SearchCapabilities", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("SortCapabilities", false, false, "string");

        stateVariable = serviceDescription->addStateVariable("SystemUpdateID", true, false, "ui4");
    }
    else
    {
        qCritical() << "invalid service description";
    }
}

bool ServiceContentDirectory::replyAction(HttpRequest *request, const SoapAction &action)
{
    if (action.actionName() == "GetSearchCapabilities")
    {
        StateVariableItem *searchCaps = findStateVariableByName("SearchCapabilities");

        if (!action.arguments().isEmpty())
        {
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }
        else if (searchCaps)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("SearchCaps", searchCaps->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable SearchCapabilities";
            UpnpError error(UpnpError::ACTION_FAILED);
            request->replyError(error);
        }

        return true;
    }

    if (action.actionName() == "GetSortCapabilities")
    {
        StateVariableItem *sortCaps = findStateVariableByName("SortCapabilities");

        if (!action.arguments().isEmpty())
        {
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }
        else if (sortCaps)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("SortCaps", sortCaps->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable SortCapabilities";
            UpnpError error(UpnpError::ACTION_FAILED);
            request->replyError(error);
        }

        return true;
    }

    if (action.actionName() == "GetSystemUpdateID")
    {
        StateVariableItem *sysUpdateId = findStateVariableByName("SystemUpdateID");

        if (!action.arguments().isEmpty())
        {
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
        }
        else if (sysUpdateId)
        {
            SoapActionResponse response(action.serviceType(), action.actionName());

            response.addArgument("Id", sysUpdateId->data(StateVariableItem::ValueRole).toString());
            request->replyAction(response);
        }
        else
        {
            qCritical() << "invalid state variable SystemUpdateID";
            UpnpError error(UpnpError::ACTION_FAILED);
            request->replyError(error);
        }

        return true;
    }

    if (action.actionName() == "Browse")
    {
        MediaRenderer* renderer = Q_NULLPTR;
        if (m_renderersModel)
            renderer = m_renderersModel->rendererFromIp(request->peerAddress().toString());

        if (!action.arguments().contains("ObjectID") || action.arguments().size()!=6)
        {
            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
            return false;
        }

        QString objectID = action.argumentValue("ObjectID");
        if (!objectID.isEmpty())
        {
            QString browseFlag = action.argumentValue("BrowseFlag");
            QString startingIndex = action.argumentValue("StartingIndex");
            QString requestedCount = action.argumentValue("RequestedCount");
            QString filter = action.argumentValue("Filter");
            QString sortCriteria = action.argumentValue("SortCriteria");

            if (!sortCriteria.isEmpty())
                qWarning() << "SORT CRITERIA" << sortCriteria;

            if (!browseFlag.isEmpty() && !startingIndex.isEmpty() && !requestedCount.isEmpty() && !filter.isEmpty())
            {
                qDebug() << "Browse" << objectID << startingIndex << requestedCount << browseFlag << filter << sortCriteria;

                if (startingIndex.toInt() < 0)
                {
                    qCritical() << "invalid startingIndex" << startingIndex.toInt();
                    UpnpError error(UpnpError::INVALID_ARGS);
                    request->replyError(error);
                    return false;
                }

                if (requestedCount.toInt() < 0)
                {
                    qCritical() << "invalid requestedCount" << requestedCount.toInt();
                    UpnpError error(UpnpError::INVALID_ARGS);
                    request->replyError(error);
                    return false;
                }

                if (browseFlag == "BrowseMetadata" && startingIndex.toInt() != 0)
                {
                    qCritical() << "invalid startingIndex" << startingIndex.toInt() << "when browseFlag is" << browseFlag;
                    UpnpError error(UpnpError::INVALID_ARGS);
                    request->replyError(error);
                    return false;
                }

                QObject context;
                QList<DlnaResource*> l_dlna = rootFolder.getDLNAResources(objectID,
                                                                          browseFlag == "BrowseDirectChildren",
                                                                          startingIndex.toInt(),
                                                                          requestedCount.toInt(),
                                                                          QString(),
                                                                          &context);

                qDebug() << "returned DLNA resources" << l_dlna.size();

                DlnaResource *object_requested = Q_NULLPTR;
                if (browseFlag == "BrowseDirectChildren")
                {
                    if (!l_dlna.isEmpty())
                        object_requested = l_dlna.at(0)->getDlnaParent();
                }
                else if (browseFlag == "BrowseMetadata")
                {
                    if (l_dlna.size() == 1)
                        object_requested = l_dlna.at(0);
                }
                else
                {
                    qCritical() << "invalid browseFlag" << browseFlag;
                    UpnpError error(UpnpError::INVALID_ARGS);
                    request->replyError(error);
                    return false;
                }

                if (object_requested == Q_NULLPTR)
                {
                    qCritical() << "invalid object" << objectID << object_requested;
                    UpnpError error(UpnpError::INVALID_OBJECT);
                    request->replyError(error);
                    return false;
                }

                DidlLite didlDoc;

                foreach (DlnaResource* resource, l_dlna)
                {
                    auto dlnaItem = qobject_cast<DlnaItem*>(resource);
                    if (dlnaItem && renderer)
                        dlnaItem->setSinkProtocol(renderer->sinkProtocols());

                    didlDoc.addElement(resource->getXmlContentDirectory(&didlDoc, filter.split(",")));
                }

                qDebug() << didlDoc.toString();

                SoapActionResponse response(action.serviceType(), action.actionName());
                response.addArgument("Result", didlDoc.toString(-1));
                response.addArgument("NumberReturned", QString("%1").arg(l_dlna.size()));

                if (browseFlag == "BrowseMetadata")
                {
                    response.addArgument("TotalMatches", QString("1"));
                }
                else if (browseFlag == "BrowseDirectChildren")
                {
                    DlnaResource *parent = Q_NULLPTR;
                    if (!l_dlna.isEmpty())
                        parent = l_dlna.at(0)->getDlnaParent();

                    if (parent)
                    {
                        response.addArgument("TotalMatches", QString("%1").arg(parent->getChildrenSize()));
                    }
                    else if (!l_dlna.isEmpty())
                    {
                        response.addArgument("TotalMatches", "0");
                    }
                    else
                    {
                        UpnpError error(UpnpError::INVALID_PROCESS_REQUEST);
                        request->replyError(error);
                        return false;
                    }
                }
                else
                {
                    qCritical() << "invalid browseFlag" << browseFlag;
                    UpnpError error(UpnpError::INVALID_ARGS);
                    request->replyError(error);
                    return false;
                }

                if (object_requested)
                    response.addArgument("UpdateID", QVariant::fromValue(object_requested->getUpdateId()).toString());
                else
                    qCritical() << "invalid object request in Browse action" << objectID;

                request->replyAction(response);
                return true;
            }

            qCritical() << "invalid argument in Browse" << objectID << startingIndex << requestedCount << browseFlag << filter << sortCriteria;

            UpnpError error(UpnpError::INVALID_ARGS);
            request->replyError(error);
            return false;
        }

        UpnpError error(UpnpError::INVALID_OBJECT);
        request->replyError(error);
        return false;
    }

    qCritical() << "unknwon action" << action.actionName();
    UpnpError error(UpnpError::INVALID_ACTION);
    request->replyError(error);
    return false;
}

bool ServiceContentDirectory::replyRequest(HttpRequest *request)
{
    if (AbstractService::replyRequest(request))
        return true;

    if ((request->operation() == QNetworkAccessManager::GetOperation  || request->operation() == QNetworkAccessManager::HeadOperation) && request->url().toString().startsWith("/get/"))
    {
        qDebug() << this << request->operationString() << request->url();

        QStringList l_path = request->url().toString().split("/");
        if (l_path.size() > 1)
        {
            const QString& objectID = l_path.at(l_path.size()-2);

            auto dlna = qobject_cast<DlnaItem*>(getDlnaResource(request->peerAddress().toString(), objectID));

            if (dlna)
            {
                MediaRenderer* renderer = Q_NULLPTR;
                if (m_renderersModel)
                    renderer = m_renderersModel->rendererFromIp(request->peerAddress().toString());

                if (renderer)
                    dlna->setSinkProtocol(renderer->sinkProtocols());

                if (request->url().fileName(QUrl::FullyEncoded).startsWith("thumbnail0000"))
                {
                    request->replyData(dlna->getByteAlbumArt(), "image/jpeg");
                }
                else
                {
                    QScopedPointer<HttpRange> range(request->range(dlna->size()));

                    qint64 timeSeekRangeStart = -1;
                    qint64 timeSeekRangeEnd = -1;
                    if (!request->header("TIMESEEKRANGE.DLNA.ORG").isEmpty())
                    {
                        readTimeSeekRange(QString("TIMESEEKRANGE.DLNA.ORG: %1").arg(request->header("TIMESEEKRANGE.DLNA.ORG")), &timeSeekRangeStart, &timeSeekRangeEnd);
                    }

                    QStringList m_header;
                    m_header << QString("Content-Type: %1").arg(dlna->mimeType());

                    if (!request->header("GETCONTENTFEATURES.DLNA.ORG").isEmpty())
                        m_header << QString("contentFeatures.dlna.org: %1").arg(dlna->getDlnaContentFeatures());

                    if (!request->header("transferMode.dlna.org").isEmpty())
                        m_header << QString("transferMode.dlna.org: %1").arg(request->header("transferMode.dlna.org"));

                    if (!request->header("GETMEDIAINFO.SEC").isEmpty())
                        m_header << QString("MediaInfo.sec: SEC_Duration=%1;").arg(dlna->getLengthInMilliSeconds());

                    if (dlna->getdlnaOrgOpFlags().at(1) == '1')
                        m_header << QString("Accept-Ranges: bytes");

                    m_header << QString("SERVER: %1").arg(request->serverName());


                    HttpRequest::HttpStatus replyStatus = HttpRequest::HTTP_200_OK;

                    if (range)
                    {
                        replyStatus = HttpRequest::HTTP_206_Partial_Content;
                        m_header << QString("Content-Range: bytes %1-%2/%3").arg(range->getStartByte()).arg(range->getEndByte()).arg(dlna->size());

                        m_header << QString("Content-Length: %1").arg(range->getLength());

                        m_header << QString("DATE: %1").arg(QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss") + " GMT");

                        if (range->getEndByte() >= range->getSize()-1)
                            request->setpartialStreaming(false);
                        else
                            request->setpartialStreaming(true);
                    }
                    else
                    {
                        m_header << QString("Content-Length: %1").arg(dlna->size());
                    }

                    if (timeSeekRangeStart >= 0 && dlna->getLengthInMilliSeconds() > 0)
                    {
                        QTime start_time(0, 0, 0);
                        start_time = start_time.addSecs(timeSeekRangeStart);

                        QTime end_time(0, 0, 0);
                        if (timeSeekRangeEnd != -1)
                        {
                            end_time = end_time.addSecs(timeSeekRangeEnd);
                        } else {
                            end_time = end_time.addMSecs(dlna->getLengthInMilliSeconds());
                        }

                        QTime length_time(0, 0, 0);
                        length_time = length_time.addMSecs(dlna->getLengthInMilliSeconds());

                        m_header << QString("TimeSeekRange.dlna.org: npt=%1-%2/%3").arg(start_time.toString("hh:mm:ss,z"), end_time.toString("hh:mm:ss,z"), length_time.toString("hh:mm:ss,z"));
                        m_header << QString("X-Seek-Range: npt=%1-%2/%3").arg(start_time.toString("hh:mm:ss,z"), end_time.toString("hh:mm:ss,z"), length_time.toString("hh:mm:ss,z"));
                        m_header << QString("X-AvailableSeekRange: 1 npt=%1-%2").arg(0).arg(dlna->getLengthInSeconds());
                    }

                    if (request->sendHeader(m_header, replyStatus))
                    {
                        if (request->operation() == QNetworkAccessManager::GetOperation)
                        {
                            if (dlna->toTranscode())
                                request->logMessage(QString("Transcode media from %1 to %2").arg(dlna->metaDataFormat(), dlna->mimeType()));
                            else
                                request->logMessage(QString("Stream media %1 %2").arg(dlna->metaDataFormat(), dlna->mimeType()));

                            request->logMessage(QString("%1 bytes to send in %2.").arg(dlna->size()).arg(QTime(0, 0).addMSecs(dlna->getLengthInMilliSeconds()).toString("hh:mm:ss.zzz")));

                            QString mediaFilename = dlna->getSystemName();
                            request->setRequestedResource(mediaFilename);
                            request->setRequestedDisplayName(dlna->getDisplayName());
                            servingMedia(mediaFilename, 0);

                            // recover resume time
                            qint64 resume = dlna->getResumeTime();
                            if (resume>0) {
                                timeSeekRangeStart = resume/1000;
                                request->setClockSending(timeSeekRangeStart*1000);
                            }

                            Device *streamContent = dlna->getStream();

                            QTcpSocket *socket = request->tcpSocket();

                            if (!streamContent || !socket)
                            {
                                // No inputStream indicates that transcoding / remuxing probably crashed.
                                QString message = QString("There is no inputstream to return for %1.").arg(dlna->getDisplayName());
                                request->logMessage(message);
                                request->setError(message);
                                request->close();

                                if (streamContent)
                                    streamContent->deleteLater();
                            }
                            else
                            {
                                if (range && !range->isNull())
                                    streamContent->setRange(range->getStartByte(), range->getEndByte());
                                else if (timeSeekRangeStart != -1 || timeSeekRangeEnd != -1)
                                    streamContent->setTimeSeek(timeSeekRangeStart, timeSeekRangeEnd);

                                connect(socket, SIGNAL(disconnected()), streamContent, SLOT(close()));

                                request->setMaxBufferSize(streamContent->maxBufferSize());

                                streamContent->moveToThread(m_streamingThread);

                                connect(streamContent, SIGNAL(readyToOpen()), this, SLOT(streamReadyToOpen()), Qt::UniqueConnection);
                                connect(streamContent, SIGNAL(openedSignal()), request, SLOT(streamOpened()));
                                connect(streamContent, SIGNAL(openedSignal()), streamContent, SLOT(startRequestData()), Qt::UniqueConnection);
                                connect(streamContent, SIGNAL(readyRead()), request, SLOT(streamDataAvailable()));
                                connect(streamContent, SIGNAL(status(QString)), request, SLOT(streamingStatus(QString)));
                                connect(streamContent, SIGNAL(LogMessage(QString)), request, SLOT(logMessage(QString)));
                                connect(streamContent, SIGNAL(errorRaised(QString)), request, SLOT(streamError(QString)));
                                connect(streamContent, SIGNAL(endReached()), request, SLOT(streamingCompleted()));
                                connect(streamContent, SIGNAL(closed()), request, SLOT(streamClosed()));
                                connect(streamContent, SIGNAL(abort()), request, SLOT(close()));

                                connect(request, SIGNAL(requestStreamingData(qint64)), streamContent, SLOT(requestData(qint64)));
                                connect(streamContent, SIGNAL(sendDataToClientSignal(QByteArray)), request, SLOT(sendPartialData(QByteArray)));

                                connect(request, &HttpRequest::servingSignal, this, &ServiceContentDirectory::servingMedia);

                                connect(request, SIGNAL(servingFinishedSignal(QString,QString,int)), this, SIGNAL(servingFinishedSignal(QString,QString,int)));
                                connect(request, SIGNAL(servingFinishedSignal(QString,QString,int)), this, SLOT(servingFinished(QString,QString,int)));

                                connect(request, SIGNAL(servingRendererSignal(QString,QString)), this, SIGNAL(servingRendererSignal(QString,QString)));

                                if (streamContent->isReadyToOpen())
                                    streamContent->open();
                            }
                        }
                    }
                }
            }
            else
            {
                request->setError(QString("invalid DLNA resource %1.").arg(objectID));
                request->close();
            }
        }

        return true;
    }

    return false;
}

void ServiceContentDirectory::_addFolder(const QString &folder)
{
    if (QFileInfo(folder).isDir())
        emit addFolderSignal(folder);
    else
        emit error_addFolder(folder);   // error folder is not a directory
}

void ServiceContentDirectory::folderAddedSlot(const QString &folder)
{
    listFolderAdded << folder;
}

void ServiceContentDirectory::reloadLibrary()
{
    emit reloadLibrarySignal(listFolderAdded);
}

void ServiceContentDirectory::streamReadyToOpen()
{
    auto stream = qobject_cast<Device*>(sender());
    stream->open();
}

void ServiceContentDirectory::readTimeSeekRange(const QString &data, qint64 *start, qint64 *end)
{
    QRegularExpression pattern(R"(timeseekrange\.dlna\.org:\s*npt\s*=\s*(\d+)\-?(\d*))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = pattern.match(data);

    if (match.hasMatch())
    {
        if (!match.captured(1).isEmpty())
            *start = match.captured(1).toLong();

        if (!match.captured(2).isEmpty())
            *end = match.captured(2).toLong();
    }
    else
    {
        qCritical() << "invalid TimeSeekRange" << data;
    }
}

void ServiceContentDirectory::servingFinished(QString host, const QString &filename, int status)
{
    Q_UNUSED(host)

    if (status == 0)
    {
        emit incrementCounterPlayedSignal(filename);

        // remove resource from cache
        qDebug() << "SERVING FINISHED" << host << filename << m_dlnaresources.keys();
        auto it = m_dlnaresources.begin();
        while (it != m_dlnaresources.end())
        {
            if (it.key().startsWith(host) && it.value()->getSystemName() == filename)
            {
                qDebug() << "remove from cache" << it.key() << it.value()->getSystemName();
                DlnaResource *resource = it.value();
                it = m_dlnaresources.erase(it);
                resource->deleteLater();
            }
            else
            {
                ++it;
            }
        }
    }
}

void ServiceContentDirectory::servingMedia(const QString &filename, int playedDurationInMs)
{
    QHash<QString, QVariant> data;
    data.insert("last_played", QDateTime::currentDateTime());
    if (playedDurationInMs>0)
        data.insert("progress_played", playedDurationInMs);
    emit updateMediaData(filename, data);
}

DlnaResource *ServiceContentDirectory::getDlnaResource(const QString &hostaddress, const QString &objId)
{
    QString dlnaresourceID = QString("%1_%2").arg(hostaddress, objId);

    if (m_dlnaresources.contains(dlnaresourceID))
        return m_dlnaresources[dlnaresourceID];

    qDebug() << "request" << objId << "from" << hostaddress;

    QObject context;
    QList<DlnaResource*> l_dlna = rootFolder.getDLNAResources(objId, false, 0, 0, "", &context);
    if (l_dlna.size() == 1)
    {
        // store dlna object in cache
        m_dlnaresources[dlnaresourceID] = l_dlna.at(0);
        l_dlna.at(0)->setParent(this);
        if (l_dlna.at(0)->getDlnaParent())
            connect(l_dlna.at(0)->getDlnaParent(), SIGNAL(dlnaContentUpdated()), this, SLOT(dlnaContentUpdated()), Qt::UniqueConnection);
        return m_dlnaresources[dlnaresourceID];
    }

    return Q_NULLPTR;
}

void ServiceContentDirectory::dlnaContentUpdated()
{
    auto dlna = qobject_cast<DlnaResource*>(sender());
    if (dlna)
    {
        // remove from cache chid object of dlna resource updated
        qDebug() << "dlna content updated" << dlna->getResourceId();
        QString id = QString("_%1").arg(dlna->getResourceId());
        auto it = m_dlnaresources.begin();
        while (it != m_dlnaresources.end())
        {
            if (it.key().contains(id))
            {
                qDebug() << "remove child" << it.key() << "from cache";
                DlnaResource *resource = it.value();
                it = m_dlnaresources.erase(it);
                resource->deleteLater();
            }
            else
            {
                ++it;
            }
        }
    }
    else
    {
        qCritical() << "invalid dlna object" << sender();
    }
}

void ServiceContentDirectory::mediaRendererDestroyed(const QString &hostaddress)
{
    qDebug() << "renderer destroyed" << hostaddress;
    auto it = m_dlnaresources.begin();
    while (it != m_dlnaresources.end())
    {
        if (it.key().startsWith(hostaddress))
        {
            qDebug() << "remove child" << it.key() << "from cache";
            DlnaResource *resource = it.value();
            it = m_dlnaresources.erase(it);
            resource->deleteLater();
        }
        else
        {
            ++it;
        }
    }
}
