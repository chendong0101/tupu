import { Anchor, Avatar, Card, Modal, AutoComplete, Table, Row, Col, Checkbox, Form, Button, message, Input, Drawer } from 'antd';
import React, { useState, useRef } from 'react';
import { useIntl, FormattedMessage } from 'umi';
import { PageContainer, FooterToolbar } from '@ant-design/pro-layout';
import ProTable, { ProColumns, ActionType } from '@ant-design/pro-table';
import ProDescriptions, { ProDescriptionsItemProps } from '@ant-design/pro-descriptions';
import UpdateForm, { FormValueType } from './components/UpdateForm';
import moment from 'moment';
import { sqlSearch } from './service';
const FormItem = Form.Item;

const formatterTime = (val) => {
    return val ? moment(val).format('YYYY-MM-DD HH:mm:ss') : ''
}

const columns = [
  {
    title: 'predicate',
    dataIndex: 'predicate',
    key: 'name',
  },
  {
    title: 'value',
    dataIndex: 'value',
    key: 'value',
    width: '300px',
    fixed: true,
  },
  {
    title: 'to_kgc_id',
    dataIndex: 'to_kgc_id',
    key: 'to_kgc_id',
    width: '100px',
    fixed: true,
  },
  {
    title: 'to_engity_id',
    dataIndex: 'to_entity_id',
    key: 'to_entity_id',
    width: '80px',
    fixed: true,
  },
  {
    title: 'source',
    dataIndex: 'source',
    key: 'source',
  },
  {
    title: 'priority',
    dataIndex: 'priority',
    key: 'priority',
  },
  {
    title: 'op',
    dataIndex: 'op',
    key: 'op',
  },
  {
    title: 'op_time',
    dataIndex: 'op_time',
    render: formatterTime,
    sorter: true,
  },
];

const TableList: React.FC<{}> = () => {

  const [form] = Form.useForm();

  const [data, setData] = useState([]);

  const [isModalVisible, setIsModalVisible] = useState(false);

  const handleSearch = async () => {
    const { getFieldValue } = form;
    console.log("form", form.getFieldValue("forceQuery"));
    console.log("form", form);
    var data = await sqlSearch({
      sql: getFieldValue("sql"),
      forceQuery: getFieldValue("forceQuery") === true ? "1" : "0",
      forwardSearch: getFieldValue("forwardSearch") === true ? "1" : "0",
      resformat: "json",
      encoding: "utf-8",
      fatResult: "1",
    });
    setData(data);
  };

  const showModal = () => {
    setIsModalVisible(true);
  };

  const handleOk = () => {
          setIsModalVisible(false);
  };

  const handleCancel = () => {
          setIsModalVisible(false);
  };

  return (
    <PageContainer>
             <Modal title="Tupu SQL ??????" width={1000} visible={isModalVisible} onOk={handleOk} onCancel={handleCancel}>
                <p>1????????? SQL </p>
                <p>SELECT * WHERE NAME = ????????? AND ?????? != "???????????????" ORDER BY ????????? LIMIT 2</p>
                <p>SELECT ?????? WHERE NAME = ????????? ORDER BY ????????? LIMIT 2</p>
                <p>SELECT * WHERE ?????? = ?????? AND ?????? = ????????? AND ?????? = ??? ORDER BY ????????? LIMIT 2</p>
                <p>2???IN ??? SQL </p>
                <p>SELECT * WHERE NAME = (?????????, ?????????, ?????????, ??????) ORDER BY ????????? LIMIT 4</p>
                <p>SELECT * WHERE ID = (6283fabd66d6fdda40eec5acbd7d5b23, 5e628ba3fe686610c40d3eb0b5b1e173)</p>
                <p>3????????? SQL </p>
                <p>SELECT * WHERE NAME = (SELECT ?????? WHERE NAME= ?????????) AND ?????? = ????????? ORDER BY ????????? LIMIT 2</p>
                <p>SELECT * WHERE ?????? = ?????? AND ???????????? = (SELECT ???????????? WHERE NAME = ?????????) ORDER BY ????????? LIMIT 5</p>
                <p>4???KL ?????????</p>
                <p>SELECT * WHERE ALIAS=?????????????????? AND entity_type = list ORDER BY ????????? LIMIT 10</p>
             </Modal>
            <Form form={form}>
            <Row type="flex" justify="space-around" align="middle">
            <FormItem id="control-param" name="forceQuery" valuePropName="checked">
                <Checkbox>????????????</Checkbox>
            </FormItem>
            <FormItem id="control-param" name="forwardSearch" valuePropName="checked">
                <Checkbox>????????????</Checkbox>
            </FormItem>
            <Button onClick={showModal}>
                Tupu SQL ??????
            </Button>
            </Row>
            <FormItem id="control-sql" name="sql" label="sql" wrapperCol={{ span: 44 }} >
                    <Input.Search type="textarea" id="control-textarea" 
                        onSearch = { handleSearch }
                        onPressEnter = { handleSearch }
                        history="SELECT * WHERE NAME = ????????? ORDER BY ????????? LIMIT 2"
                        allowClear
                        enterButton="Search"
                        size="large"
                    />
            </FormItem>
            </Form>
            <p> ?????? {data.length} ????????? </p>
            {
               Array.isArray(data) && 
                data.map(entity => {
                    var icon = "";
                    var baike = "";
                    entity.triads.forEach(triad => {
                        if (triad.predicate === "??????") {
                            icon = triad.value;
                        }
                        if (triad.predicate === "????????????") {
                            baike = triad.value;
                        }
                    });
                    return (
                        <Row type="flex" justify="space-around" align="middle">
                        <Col flex="1400px">
                            <Card style={{ width: 1400, marginTop: 20}} justify="space-around" >
                              <Card.Meta
                                avatar={
                                    <Avatar src={icon} size="large"/>
                                }
                                title=<Anchor> <Anchor.Link href = {baike} title = {"kgc_id: " + entity.kgc_id}/></Anchor>
                                description={"entity_id: " + entity.entity_id}
                              />
                            <Table dataSource={entity.triads} columns={columns} />
                            </Card>
                        </Col>
                        </Row>
                    );
                })
            }
    </PageContainer>
  );
};

export default TableList;
