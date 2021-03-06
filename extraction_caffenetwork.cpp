#include "extraction_caffenetwork.h"

ExtractionCaffeNetwork::ExtractionCaffeNetwork()
{
    this->networkLoaded = false;
}

ExtractionCaffeNetwork::~ExtractionCaffeNetwork()
{
    if (this->networkLoaded) {
        net_.reset();
        mean_.release();
    }
}

bool ExtractionCaffeNetwork::caffePairCompare(const std::pair<float, int>& lhs, const std::pair<float, int>& rhs)
{
    return lhs.first > rhs.first;
}

/* Return the indices of the top N values of vector v. */
std::vector<int> ExtractionCaffeNetwork::caffeArgmax(const std::vector<float>& v, int N)
{
    std::vector<std::pair<float, int> > pairs;
    for (size_t i = 0; i < v.size(); ++i)
        pairs.push_back(std::make_pair(v[i], i));
    std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), caffePairCompare);

    std::vector<int> result;
    for (int i = 0; i < N; ++i)
        result.push_back(pairs[i].second);
    return result;
}

std::vector<Prediction> ExtractionCaffeNetwork::classify(const cv::Mat img)
{
    int N = 5;
    std::vector<float> output = this->predict(img);

    N = std::min<int>(labels_.size(), N);
    std::vector<int> maxN = caffeArgmax(output, N);
    std::vector<Prediction> predictions;
    for (int i = 0; i < N; ++i) {
        int idx = maxN[i];
        predictions.push_back(std::make_pair(labels_[idx], output[idx]));
    }

    return predictions;
}

void ExtractionCaffeNetwork::loadModel(const QString &model_file,
                             const QString &trained_file,
                             const QString &mean_file,
                             const QString &label_file)
{
#ifdef CPU_ONLY
    Caffe::set_mode(Caffe::CPU);
#else
    Caffe::set_mode(Caffe::GPU);
#endif

    /* Load the network. */
    net_.reset(new Net<float>(model_file.toStdString(), TEST));
    net_->CopyTrainedLayersFrom(trained_file.toStdString());

    CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
    CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

    Blob<float>* input_layer = net_->input_blobs()[0];
    num_channels = input_layer->channels();
    CHECK(num_channels == 3 || num_channels == 1) << "Input layer should have 1 or 3 channels.";
    input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

    /* Load the binaryproto mean file. */
    this->setMean(mean_file.toStdString());

    /* Load labels. */
    std::ifstream labels(label_file.toStdString().c_str());
    CHECK(labels) << "Unable to open labels file " << label_file.toStdString();
    std::string line;
    while (std::getline(labels, line))
        labels_.push_back(string(line));

    Blob<float>* output_layer = net_->output_blobs()[0];
    CHECK_EQ(labels_.size(), output_layer->channels()) << "Number of labels is different from the output layer dimension.";

    this->networkLoaded = true;
}

bool ExtractionCaffeNetwork::getNetworkLoaded() const
{
    return networkLoaded;
}

void ExtractionCaffeNetwork::setMean(const std::string& mean_file)
{
    BlobProto blob_proto;
    ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

    /* Convert from BlobProto to Blob<float> */
    Blob<float> mean_blob;
    mean_blob.FromProto(blob_proto);
    CHECK_EQ(mean_blob.channels(), num_channels)
            << "Number of channels of mean file doesn't match input layer.";

    /* The format of the mean file is planar 32-bit float BGR or grayscale. */
    std::vector<cv::Mat> channels;
    float* data = mean_blob.mutable_cpu_data();
    for (int i = 0; i < num_channels; ++i) {
        /* Extract an individual channel. */
        cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
        channels.push_back(channel);
        data += mean_blob.height() * mean_blob.width();
    }

    /* Merge the separate channels into a single image. */
    cv::Mat mean;
    cv::merge(channels, mean);

    /* Compute the global mean pixel value and create a mean image
    * filled with this value. */
    cv::Scalar channel_mean = cv::mean(mean);
    mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
}

std::vector<float> ExtractionCaffeNetwork::predict(const cv::Mat& img)
{
    Blob<float>* input_layer = this->net_->input_blobs()[0];
    input_layer->Reshape(1, this->num_channels,
                         this->input_geometry_.height,
                         this->input_geometry_.width);

    /* Forward dimension change to all layers. */
    this->net_->Reshape();

    std::vector<cv::Mat> input_channels;
    this->wrapInputLayer(&input_channels);

    this->preprocess(img, &input_channels);
    Blob<float>* output_layer = this->net_->output_blobs()[0];
    const float* begin = output_layer->cpu_data();
    const float* end = begin + output_layer->channels();

    net_->Forward();

    /* Copy the output layer to a std::vector */
    output_layer = this->net_->output_blobs()[0];
    begin = output_layer->cpu_data();
    end = begin + output_layer->channels();

    return std::vector<float>(begin, end);
}

void ExtractionCaffeNetwork::wrapInputLayer(std::vector<cv::Mat>* input_channels)
{
    Blob<float>* input_layer = net_->input_blobs()[0];

    int width = input_layer->width();
    int height = input_layer->height();
    float* input_data = input_layer->mutable_cpu_data();
    for (int i = 0; i < input_layer->channels(); ++i) {
        cv::Mat channel(height, width, CV_32FC1, input_data);
        input_channels->push_back(channel);
        input_data += width * height;
    }
}

void ExtractionCaffeNetwork::preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels)
{
    /* Convert the input image to the input image format of the network. */
    cv::Mat sample;
    if (img.channels() == 3 && num_channels == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
    else if (img.channels() == 4 && num_channels == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
    else if (img.channels() == 4 && num_channels == 3)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
    else if (img.channels() == 1 && num_channels == 3)
        cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
    else
        sample = img;

    cv::Mat sample_resized;

    if (sample.size() != input_geometry_)
        cv::resize(sample, sample_resized, input_geometry_);
    else
        sample_resized = sample;

    cv::Mat sample_float;
    if (num_channels == 3)
        sample_resized.convertTo(sample_float, CV_32FC3);
    else
        sample_resized.convertTo(sample_float, CV_32FC1);

    cv::Mat sample_normalized;
    cv::subtract(sample_float, mean_, sample_normalized);

    /* This operation will write the separate BGR planes directly to the
    * input layer of the network because it is wrapped by the cv::Mat
    * objects in input_channels. */
    sample_normalized /= 255.0;
    cv::split(sample_normalized, *input_channels);

    CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
          == net_->input_blobs()[0]->cpu_data())
            << "Input channels are not wrapping the input layer of the network.";
}

void ExtractionCaffeNetwork::preprocessBatch(const vector<cv::Mat> imgs, std::vector< std::vector<cv::Mat> >* input_batch)
{
    for (int i = 0 ; i < imgs.size(); i++){
        cv::Mat img = imgs[i];
        std::vector<cv::Mat> *input_channels = &(input_batch->at(i));

        /* Convert the input image to the input image format of the network. */
        cv::Mat sample;
        if (img.channels() == 3 && this->num_channels == 1)
          cv::cvtColor(img, sample, CV_BGR2GRAY);
        else if (img.channels() == 4 && this->num_channels == 1)
          cv::cvtColor(img, sample, CV_BGRA2GRAY);
        else if (img.channels() == 4 && this->num_channels == 3)
          cv::cvtColor(img, sample, CV_BGRA2BGR);
        else if (img.channels() == 1 && this->num_channels == 3)
          cv::cvtColor(img, sample, CV_GRAY2BGR);
        else
          sample = img;

        cv::Mat sample_resized;
        if (sample.size() != this->input_geometry_)
          cv::resize(sample, sample_resized, this->input_geometry_);
        else
          sample_resized = sample;

        cv::Mat sample_float;
        if (this->num_channels == 3)
          sample_resized.convertTo(sample_float, CV_32FC3);
        else
          sample_resized.convertTo(sample_float, CV_32FC1);

        cv::Mat sample_normalized;

        cv::subtract(sample_float, this->mean_, sample_normalized);

        /* This operation will write the separate BGR planes directly to the
         * input layer of the network because it is wrapped by the cv::Mat
         * objects in input_channels. */
        sample_normalized /= 255.0;
        cv::split(sample_normalized, *input_channels);

//        CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
//              == net_->input_blobs()[0]->cpu_data())
//          << "Input channels are not wrapping the input layer of the network.";
    }
}

void ExtractionCaffeNetwork::wrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch)
{
    Blob<float>* input_layer_ = this->net_->input_blobs()[0];

    int width = input_layer_->width();
    int height = input_layer_->height();
    int num = input_layer_->num();
    float* input_data = input_layer_->mutable_cpu_data();
    for ( int j = 0; j < num; j++){
        vector<cv::Mat> input_channels;
        for (int i = 0; i < input_layer_->channels(); ++i){
          cv::Mat channel(height, width, CV_32FC1, input_data);
          input_channels.push_back(channel);
          input_data += width * height;
        }
        input_batch -> push_back(vector<cv::Mat>(input_channels));
    }
}

std::vector<float> ExtractionCaffeNetwork::predictBatch(const std::vector< cv::Mat > imgs)
{
    Blob<float>* input_layer = this->net_->input_blobs()[0];
    input_layer->Reshape(imgs.size(), this->num_channels,
                         this->input_geometry_.height,
                         this->input_geometry_.width);

    /* Forward dimension change to all layers. */
    this->net_->Reshape();

    std::vector<std::vector<cv::Mat>> input_batch;
    this->wrapBatchInputLayer(&input_batch);

    this->preprocessBatch(imgs, &input_batch);

    Blob<float>* output_layer = this->net_->output_blobs()[0];
    const float* begin = output_layer->cpu_data();
    const float* end = begin + output_layer->channels()*imgs.size();

    this->net_->Forward();

    /* Copy the output layer to a std::vector */
    output_layer = this->net_->output_blobs()[0];
    begin = output_layer->cpu_data();
    end = begin + output_layer->channels()*imgs.size();

    return std::vector<float>(begin, end);
}

std::vector<std::vector<Prediction>> ExtractionCaffeNetwork::classifyBatch(const std::vector< cv::Mat > imgs, int num_classes)
{
    std::vector<float> output_batch = this->predictBatch(imgs);

    std::vector< std::vector<Prediction> > predictions;
    for(int j = 0; j < imgs.size(); j++){
        std::vector<float> output(output_batch.begin() + j*num_classes, output_batch.begin() + (j+1)*num_classes); //rozdiel
        std::vector<int> maxN = caffeArgmax(output, num_classes);
        std::vector<Prediction> prediction_single;
        for (int i = 0; i < num_classes; ++i) {
              int idx = maxN[i];
              prediction_single.push_back(std::make_pair(this->labels_[idx], output[idx]));
        }
        predictions.push_back(std::vector<Prediction>(prediction_single));
        output_batch.clear();
    }
    return predictions;
}

std::vector<string> ExtractionCaffeNetwork::get_stages_from_flags()
{
    string flag_stage = "";

    vector<string> stages;
    boost::split(stages, flag_stage, boost::is_any_of(","));
    return stages;
}

caffe::SolverAction::Enum ExtractionCaffeNetwork::GetRequestedAction(const std::string& flag_value)
{
    if (flag_value == "stop") {
        return caffe::SolverAction::STOP;
    }
    if (flag_value == "snapshot") {
        return caffe::SolverAction::SNAPSHOT;
    }
    if (flag_value == "none") {
        return caffe::SolverAction::NONE;
    }
    emit logSignal("trainer", "Invalid signal effect \"" + QString::fromStdString(flag_value) + "\" was specified");
}

void ExtractionCaffeNetwork::CopyLayers(caffe::Solver<float>* solver, const std::string& model_list)
{
    std::vector<std::string> model_names;
    boost::split(model_names, model_list, boost::is_any_of(",") );
    for (int i = 0; i < model_names.size(); ++i) {
        emit logSignal("trainer", "Finetuning from " + QString::fromStdString(model_names[i]));
        solver->net()->CopyTrainedLayersFrom(model_names[i]);
        for (int j = 0; j < solver->test_nets().size(); ++j) {
            solver->test_nets()[j]->CopyTrainedLayersFrom(model_names[i]);
        }
    }
}
