import { request } from 'umi';
import { SqlParams, TableListParams, TableListItem } from './data.d';

export async function sqlSearch(params: SqlParams) {
  console.log(params);
  return request('http://sqlserver01.tupu.hb.ted:38099/tupu/sql', {
    params
  });
}

export async function removeRule(params: { key: number[] }) {
  return request('/api/rule', {
    method: 'POST',
    data: {
      ...params,
      method: 'delete',
    },
  });
}

export async function addRule(params: TableListItem) {
  return request('/api/rule', {
    method: 'POST',
    data: {
      ...params,
      method: 'post',
    },
  });
}

export async function updateRule(params: TableListParams) {
  return request('/api/rule', {
    method: 'POST',
    data: {
      ...params,
      method: 'update',
    },
  });
}
